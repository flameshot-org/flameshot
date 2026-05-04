#include <QtGlobal>

#if defined(Q_OS_LINUX)

#include "waylandportalcapturebackend.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QMutexLocker>
#include <QSettings>
#include <QThread>

#include <unistd.h>
#include <cstring>

extern "C" {
#include <pipewire/pipewire.h>
#include <spa/param/video/format-utils.h>
#include <spa/param/format-utils.h>
#include <spa/utils/result.h>
#include <spa/buffer/meta.h>
}

static const char* kPortalService = "org.freedesktop.portal.Desktop";
static const char* kRemoteDesktopPath = "/org/freedesktop/portal/desktop";
static const char* kRemoteDesktopInterface = "org.freedesktop.portal.RemoteDesktop";
static const char* kScreenCastInterface = "org.freedesktop.portal.ScreenCast";

namespace {
constexpr uint SOURCE_MONITOR = 1;
constexpr uint CURSOR_EMBEDDED = 2;

constexpr uint DEVICE_KEYBOARD = 1;
constexpr uint DEVICE_POINTER = 2;

constexpr uint AXIS_VERTICAL = 0;

static const pw_registry_events kRegistryEvents = {
    PW_VERSION_REGISTRY_EVENTS,
    WaylandPortalCaptureBackend::onRegistryGlobal,
    WaylandPortalCaptureBackend::onRegistryGlobalRemove
};

static const pw_stream_events kStreamEvents = {
    PW_VERSION_STREAM_EVENTS,
    nullptr, // destroy
    WaylandPortalCaptureBackend::onStreamStateChanged,
    nullptr, // control_info
    nullptr, // io_changed
    WaylandPortalCaptureBackend::onStreamParamChanged,
    nullptr, // add_buffer
    nullptr, // remove_buffer
    WaylandPortalCaptureBackend::onStreamProcess,
    nullptr, // drained
    nullptr, // command
    nullptr  // trigger_done
};

static QImage cropToRect(const QImage& img, const QRect& rect)
{
    if (img.isNull()) {
        return {};
    }

    if (!rect.isValid()) {
        return img;
    }

    const QRect bounded = rect.intersected(img.rect());
    if (!bounded.isValid() || bounded.isEmpty()) {
        return {};
    }

    return img.copy(bounded);
}
} // namespace

WaylandPortalCaptureBackend::WaylandPortalCaptureBackend(QObject* parent)
  : QObject(parent)
{
    qRegisterMetaType<PortalStream>("PortalStream");
    qRegisterMetaType<QList<PortalStream>>("QList<PortalStream>");
    qDBusRegisterMetaType<PortalStream>();
    qDBusRegisterMetaType<QList<PortalStream>>();
}

WaylandPortalCaptureBackend::~WaylandPortalCaptureBackend()
{
    shutdown();
}

bool WaylandPortalCaptureBackend::initialize(const QString& parentWindowId)
{
    qDebug() << "initialize() - start";

    m_parentWindowId = parentWindowId;
    loadRestoreToken();

    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "No connection to the D-Bus session bus.";
        return false;
    }
    qDebug() << "DBus OK";

    if (!createRemoteDesktopSession()) {
        qWarning() << "createRemoteDesktopSession() failed";
        return false;
    }
    qDebug() << "createRemoteDesktopSession() OK";

    if (!selectDevicesAndSources()) {
        qWarning() << "selectDevicesAndSources() failed";
        shutdown();
        return false;
    }
    qDebug() << "selectDevicesAndSources() OK";

    if (!startSession(parentWindowId)) {
        qWarning() << "startSession() failed";
        shutdown();
        return false;
    }
    qDebug() << "startSession() OK. nodeId =" << m_streamNodeId;

    if (!openPipeWireRemote()) {
        qWarning() << "openPipeWireRemote() failed";
        shutdown();
        return false;
    }
    qDebug() << "openPipeWireRemote() OK. fd =" << m_pipewireFd;

    if (!startPipeWireStream()) {
        qWarning() << "startPipeWireStream() failed";
        shutdown();
        return false;
    }
    qDebug() << "startPipeWireStream() OK";

    if (!waitForFirstFrame(3000)) {
        qWarning() << "First frame did not arrive within the expected time.";
        shutdown();
        return false;
    }

    m_ready = true;
    qDebug() << "initialize() - ready";
    return true;
}

QImage WaylandPortalCaptureBackend::latestFrame() const
{
    QMutexLocker locker(&m_frameMutex);
    return cropToRect(m_latestFrame, m_selectedRect);
}

bool WaylandPortalCaptureBackend::scrollDown(int steps)
{
    if (!m_ready) {
        return false;
    }

    return callRemoteDesktopNotifyPointerAxisDiscrete(AXIS_VERTICAL, steps);
}

void WaylandPortalCaptureBackend::shutdown()
{
    m_ready = false;
    stopPipeWire();

    if (!m_sessionHandle.isEmpty()) {
        QDBusInterface sessionIface(
          kPortalService,
          m_sessionHandle,
          "org.freedesktop.portal.Session",
          QDBusConnection::sessionBus());

        sessionIface.call("Close");
        m_sessionHandle.clear();
    }

    m_streamNodeId = 0;

    {
        QMutexLocker locker(&m_frameMutex);
        m_latestFrame = {};
    }
}

bool WaylandPortalCaptureBackend::isReady() const
{
    return m_ready;
}

QRect WaylandPortalCaptureBackend::selectedRect() const
{
    return m_selectedRect;
}

void WaylandPortalCaptureBackend::setSelectedRect(const QRect& rect)
{
    m_selectedRect = rect.normalized();
    qDebug() << "Original rect =" << rect << "Final rect =" << m_selectedRect;
}

void WaylandPortalCaptureBackend::loadRestoreToken()
{
    QSettings settings("flameshot", "wayland");
    m_restoreToken = settings.value("restore_token").toString();

    if (!m_restoreToken.isEmpty()) {
        qDebug() << "restore_token loaded.";
    }
}

void WaylandPortalCaptureBackend::saveRestoreToken() const
{
    QSettings settings("flameshot", "wayland");

    if (m_restoreToken.isEmpty()) {
        settings.remove("restore_token");
        return;
    }

    settings.setValue("restore_token", m_restoreToken);
    qDebug() << "restore_token saved.";
}

void WaylandPortalCaptureBackend::clearRestoreToken()
{
    m_restoreToken.clear();

    QSettings settings("flameshot", "wayland");
    settings.remove("restore_token");

    qDebug() << "restore_token cleared.";
}

bool WaylandPortalCaptureBackend::createRemoteDesktopSession()
{
    QDBusInterface iface(
      kPortalService,
      kRemoteDesktopPath,
      kRemoteDesktopInterface,
      QDBusConnection::sessionBus());

    QVariantMap options;
    options.insert("handle_token", PortalRequestHelper::makeHandleToken("rd_create"));
    options.insert("session_handle_token", PortalRequestHelper::makeHandleToken("rd_session"));

    QDBusReply<QDBusObjectPath> reply = iface.call("CreateSession", options);
    if (!reply.isValid()) {
        qWarning() << "CreateSession failed:" << reply.error().message();
        return false;
    }

    PortalRequestHelper helper;
    auto response = helper.waitForResponse(reply.value().path());
    if (!response || response->responseCode != 0) {
        qWarning() << "CreateSession was rejected or timed out.";
        return false;
    }

    const QString sessionHandle = response->results.value("session_handle").toString();
    if (sessionHandle.isEmpty()) {
        qWarning() << "No session_handle received in CreateSession.";
        return false;
    }

    m_sessionHandle = sessionHandle;
    return true;
}

bool WaylandPortalCaptureBackend::selectDevicesAndSources()
{
    QDBusInterface remoteIface(
      kPortalService,
      kRemoteDesktopPath,
      kRemoteDesktopInterface,
      QDBusConnection::sessionBus());

    QVariantMap deviceOptions;
    deviceOptions.insert("handle_token", PortalRequestHelper::makeHandleToken("rd_devices"));
    deviceOptions.insert("types", static_cast<uint>(DEVICE_POINTER | DEVICE_KEYBOARD));

    QDBusReply<QDBusObjectPath> devicesReply =
      remoteIface.call("SelectDevices", QDBusObjectPath(m_sessionHandle), deviceOptions);

    if (!devicesReply.isValid()) {
        qWarning() << "SelectDevices failed:" << devicesReply.error().message();
        return false;
    }

    PortalRequestHelper devicesHelper;
    auto devicesResponse = devicesHelper.waitForResponse(devicesReply.value().path());
    if (!devicesResponse || devicesResponse->responseCode != 0) {
        qWarning() << "SelectDevices was rejected or timed out.";
        return false;
    }

    QDBusInterface screenIface(
      kPortalService,
      kRemoteDesktopPath,
      kScreenCastInterface,
      QDBusConnection::sessionBus());

    QVariantMap sourceOptions;
    sourceOptions.insert("handle_token", PortalRequestHelper::makeHandleToken("rd_sources"));
    sourceOptions.insert("types", static_cast<uint>(SOURCE_MONITOR));
    sourceOptions.insert("multiple", false);
    sourceOptions.insert("cursor_mode", static_cast<uint>(CURSOR_EMBEDDED));

    if (!m_restoreToken.isEmpty()) {
        sourceOptions.insert("restore_token", m_restoreToken);
        qDebug() << "Using restore_token for SelectSources.";
    }

    QDBusReply<QDBusObjectPath> sourcesReply =
      screenIface.call("SelectSources", QDBusObjectPath(m_sessionHandle), sourceOptions);

    if (!sourcesReply.isValid()) {
        qWarning() << "SelectSources failed:" << sourcesReply.error().message();
        return false;
    }

    PortalRequestHelper sourcesHelper;
    auto sourcesResponse = sourcesHelper.waitForResponse(sourcesReply.value().path());
    if (!sourcesResponse || sourcesResponse->responseCode != 0) {
        qWarning() << "SelectSources was rejected or timed out.";
        return false;
    }

    return true;
}

bool WaylandPortalCaptureBackend::startSession(const QString& parentWindowId)
{
    qDebug() << "startSession() - start";

    QDBusInterface iface(
      kPortalService,
      kRemoteDesktopPath,
      kRemoteDesktopInterface,
      QDBusConnection::sessionBus());

    QVariantMap options;
    options.insert("handle_token", PortalRequestHelper::makeHandleToken("rd_start"));

    QDBusReply<QDBusObjectPath> reply =
      iface.call("Start", QDBusObjectPath(m_sessionHandle), parentWindowId, options);

    if (!reply.isValid()) {
        qWarning() << "Start failed:" << reply.error().message();
        return false;
    }

    qDebug() << "Start DBus OK. Request path =" << reply.value().path();

    PortalRequestHelper helper;
    auto response = helper.waitForResponse(reply.value().path());
    if (!response || response->responseCode != 0) {
        qWarning() << "Start was rejected or timed out.";
        clearRestoreToken();
        return false;
    }

    qDebug() << "Start response code =" << response->responseCode;
    qDebug() << "Start results keys =" << response->results.keys();

    const QVariant streamsVariant = response->results.value("streams");
    qDebug() << "streamsVariant valid =" << streamsVariant.isValid()
             << "type =" << streamsVariant.typeName();

    if (!streamsVariant.isValid()) {
        qWarning() << "Start did not return any streams.";
        return false;
    }

    const QDBusArgument dbusArg = qvariant_cast<QDBusArgument>(streamsVariant);
    QList<PortalStream> streams;
    dbusArg >> streams;

    qDebug() << "Stream count =" << streams.size();

    if (streams.isEmpty()) {
        qWarning() << "Portal did not return any streams.";
        return false;
    }

    qDebug() << "First stream nodeId =" << streams.first().nodeId;

    m_streamNodeId = streams.first().nodeId;

    const QString restoreToken = response->results.value("restore_token").toString();
    if (!restoreToken.isEmpty()) {
        m_restoreToken = restoreToken;
        saveRestoreToken();
        qDebug() << "restore_token received and saved.";
    }

    qDebug() << "startSession() - done, nodeId =" << m_streamNodeId;
    return m_streamNodeId != 0;
}

bool WaylandPortalCaptureBackend::openPipeWireRemote()
{
    if (m_sessionHandle.isEmpty()) {
        return false;
    }

    if (!(QDBusConnection::sessionBus().connectionCapabilities() &
          QDBusConnection::UnixFileDescriptorPassing)) {
        qWarning() << "D-Bus connection does not support file descriptor passing.";
        return false;
    }

    QDBusInterface iface(
      kPortalService,
      kRemoteDesktopPath,
      kScreenCastInterface,
      QDBusConnection::sessionBus());

    QVariantMap options;
    QDBusReply<QDBusUnixFileDescriptor> reply =
      iface.call("OpenPipeWireRemote", QDBusObjectPath(m_sessionHandle), options);

    if (!reply.isValid()) {
        qWarning() << "OpenPipeWireRemote failed:" << reply.error().message();
        return false;
    }

    const QDBusUnixFileDescriptor fd = reply.value();
    if (!fd.isValid()) {
        qWarning() << "OpenPipeWireRemote returned an invalid FD.";
        return false;
    }

    m_pipewireFd = ::dup(fd.fileDescriptor());
    if (m_pipewireFd < 0) {
        qWarning() << "dup() failed for the PipeWire FD.";
        return false;
    }

    return true;
}

bool WaylandPortalCaptureBackend::startPipeWireStream()
{
    qDebug() << "startPipeWireStream() - start";
    qDebug() << "m_pipewireFd =" << m_pipewireFd
             << "m_streamNodeId =" << m_streamNodeId;

    if (m_pipewireFd < 0 || m_streamNodeId == 0) {
        qWarning() << "PipeWire has no valid FD or nodeId.";
        return false;
    }

    pw_init(nullptr, nullptr);

    m_pwLoop = pw_thread_loop_new("flameshot-wayland-loop", nullptr);
    if (!m_pwLoop) {
        qWarning() << "pw_thread_loop_new failed.";
        return false;
    }
    qDebug() << "pw_thread_loop_new OK";

    if (pw_thread_loop_start(m_pwLoop) < 0) {
        qWarning() << "pw_thread_loop_start failed.";
        return false;
    }
    qDebug() << "pw_thread_loop_start OK";

    pw_thread_loop_lock(m_pwLoop);

    pw_loop* loop = pw_thread_loop_get_loop(m_pwLoop);
    m_pwContext = pw_context_new(loop, nullptr, 0);
    if (!m_pwContext) {
        qWarning() << "pw_context_new failed.";
        pw_thread_loop_unlock(m_pwLoop);
        return false;
    }
    qDebug() << "pw_context_new OK";

    m_pwCore = pw_context_connect_fd(m_pwContext, m_pipewireFd, nullptr, 0);
    if (!m_pwCore) {
        qWarning() << "pw_context_connect_fd failed.";
        pw_thread_loop_unlock(m_pwLoop);
        return false;
    }
    qDebug() << "pw_context_connect_fd OK";

    m_pwRegistry = static_cast<pw_registry*>(
      pw_core_get_registry(m_pwCore, PW_VERSION_REGISTRY, 0));

    if (!m_pwRegistry) {
        qWarning() << "pw_core_get_registry failed.";
        pw_thread_loop_unlock(m_pwLoop);
        return false;
    }
    qDebug() << "pw_core_get_registry OK";

    pw_registry_add_listener(
      m_pwRegistry,
      &m_registryHook,
      &kRegistryEvents,
      this);

    qDebug() << "pw_registry_add_listener OK, waiting for nodeId =" << m_streamNodeId;

    pw_thread_loop_unlock(m_pwLoop);

    for (int i = 0; i < 100; ++i) {
        if (m_pwStream) {
            qDebug() << "m_pwStream created successfully";
            return true;
        }
        QThread::msleep(50);
    }

    qWarning() << "Failed to link PipeWire stream to nodeId" << m_streamNodeId;
    return false;
}

void WaylandPortalCaptureBackend::stopPipeWire()
{
    if (m_pwLoop) {
        pw_thread_loop_lock(m_pwLoop);
    }

    if (m_pwStream) {
        pw_stream_destroy(m_pwStream);
        m_pwStream = nullptr;
    }

    if (m_pwRegistry) {
        pw_proxy_destroy(reinterpret_cast<pw_proxy*>(m_pwRegistry));
        m_pwRegistry = nullptr;
    }

    if (m_pwCore) {
        pw_core_disconnect(m_pwCore);
        m_pwCore = nullptr;
    }

    if (m_pwContext) {
        pw_context_destroy(m_pwContext);
        m_pwContext = nullptr;
    }

    if (m_pwLoop) {
        pw_thread_loop_unlock(m_pwLoop);
        pw_thread_loop_stop(m_pwLoop);
        pw_thread_loop_destroy(m_pwLoop);
        m_pwLoop = nullptr;
    }

    if (m_pipewireFd >= 0) {
        ::close(m_pipewireFd);
        m_pipewireFd = -1;
    }
}

void WaylandPortalCaptureBackend::onRegistryGlobal(void* data,
                                                   uint32_t id,
                                                   uint32_t,
                                                   const char* type,
                                                   uint32_t version,
                                                   const struct spa_dict*)
{
    auto* self = static_cast<WaylandPortalCaptureBackend*>(data);
    if (!self) {
        return;
    }

    self->handleRegistryGlobal(id, type, version);
}

void WaylandPortalCaptureBackend::onRegistryGlobalRemove(void*, uint32_t)
{
}

void WaylandPortalCaptureBackend::onStreamStateChanged(void*,
                                                       enum pw_stream_state oldState,
                                                       enum pw_stream_state state,
                                                       const char* error)
{
    qDebug() << "PipeWire stream state changed:"
             << static_cast<int>(oldState)
             << "->"
             << static_cast<int>(state)
             << (error ? error : "");
}

void WaylandPortalCaptureBackend::onStreamParamChanged(void*, uint32_t, const struct spa_pod*)
{
}

void WaylandPortalCaptureBackend::onStreamProcess(void* data)
{
    auto* self = static_cast<WaylandPortalCaptureBackend*>(data);
    if (!self) {
        return;
    }

    self->handleStreamProcess();
}

void WaylandPortalCaptureBackend::handleRegistryGlobal(uint32_t id,
                                                       const char* type,
                                                       uint32_t version)
{
    qDebug() << "Registry global: id =" << id
             << "type =" << (type ? type : "(null)")
             << "version =" << version
             << "expected nodeId =" << m_streamNodeId;

    if (id != m_streamNodeId || !type) {
        return;
    }

    if (std::strcmp(type, PW_TYPE_INTERFACE_Node) != 0) {
        qDebug() << "ID matches but is not a Node type";
        return;
    }

    if (m_pwStream) {
        qDebug() << "m_pwStream already exists";
        return;
    }

    qDebug() << "Target node found in registry, creating stream...";

    pw_properties* props = pw_properties_new(
      PW_KEY_MEDIA_TYPE, "Video",
      PW_KEY_MEDIA_CATEGORY, "Capture",
      PW_KEY_MEDIA_ROLE, "Screen",
      nullptr);

    m_pwStream = pw_stream_new(m_pwCore, "flameshot-wayland-capture", props);
    if (!m_pwStream) {
        qWarning() << "pw_stream_new failed.";
        return;
    }

    pw_stream_add_listener(
      m_pwStream,
      &m_streamHook,
      &kStreamEvents,
      this);

    uint8_t buffer[1024];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    spa_video_info_raw videoInfo;
    std::memset(&videoInfo, 0, sizeof(videoInfo));

    videoInfo.format = SPA_VIDEO_FORMAT_BGRx;

    const spa_pod* params[1];
    params[0] = spa_format_video_raw_build(&builder, SPA_PARAM_EnumFormat, &videoInfo);

    const int res = pw_stream_connect(
      m_pwStream,
      PW_DIRECTION_INPUT,
      m_streamNodeId,
      static_cast<pw_stream_flags>(
        PW_STREAM_FLAG_AUTOCONNECT |
        PW_STREAM_FLAG_MAP_BUFFERS),
      params,
      1);

    if (res < 0) {
        qWarning() << "pw_stream_connect failed:" << spa_strerror(res);
        pw_stream_destroy(m_pwStream);
        m_pwStream = nullptr;
        return;
    }

    qDebug() << "pw_stream_connect OK, activating stream...";

    const int activeRes = pw_stream_set_active(m_pwStream, true);
    if (activeRes < 0) {
        qWarning() << "pw_stream_set_active failed:" << spa_strerror(activeRes);
    } else {
        qDebug() << "pw_stream_set_active OK";
    }
}

bool WaylandPortalCaptureBackend::hasValidFrame() const
{
    QMutexLocker locker(&m_frameMutex);
    return !m_latestFrame.isNull();
}

bool WaylandPortalCaptureBackend::waitForFirstFrame(int timeoutMs)
{
    qDebug() << "Waiting for first frame... timeout =" << timeoutMs << "ms";

    const int stepMs = 50;
    int waited = 0;

    while (waited < timeoutMs) {
        if (hasValidFrame()) {
            qDebug() << "First frame received successfully.";
            return true;
        }

        QThread::msleep(stepMs);
        waited += stepMs;
    }

    return false;
}

void WaylandPortalCaptureBackend::handleStreamProcess()
{
    if (!m_pwStream) {
        return;
    }

    pw_buffer* buffer = pw_stream_dequeue_buffer(m_pwStream);
    if (!buffer || !buffer->buffer) {
        return;
    }

    spa_buffer* spaBuf = buffer->buffer;
    if (spaBuf->n_datas < 1 || !spaBuf->datas[0].data) {
        pw_stream_queue_buffer(m_pwStream, buffer);
        return;
    }

    const spa_data& data = spaBuf->datas[0];

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t stride = 0;

    if (spaBuf->datas[0].chunk) {
        stride = spaBuf->datas[0].chunk->stride;
        const uint32_t size = spaBuf->datas[0].chunk->size;

        if (stride > 0 && size >= stride) {
            height = size / stride;
            if (height > 0) {
                width = stride / 4;
            }
        }
    }

    if (width == 0 || height == 0) {
        pw_stream_queue_buffer(m_pwStream, buffer);
        return;
    }

    if (stride == 0) {
        stride = width * 4;
    }

    QImage frame(
      static_cast<const uchar*>(data.data),
      static_cast<int>(width),
      static_cast<int>(height),
      static_cast<int>(stride),
      QImage::Format_RGB32);

    if (!frame.isNull()) {
        QMutexLocker locker(&m_frameMutex);
        m_latestFrame = frame.copy();

        qDebug() << "Frame received:" << width << "x" << height;
    }

    pw_stream_queue_buffer(m_pwStream, buffer);
}

bool WaylandPortalCaptureBackend::callRemoteDesktopNotifyPointerAxisDiscrete(uint axis, int steps)
{
    QDBusInterface iface(
      kPortalService,
      kRemoteDesktopPath,
      kRemoteDesktopInterface,
      QDBusConnection::sessionBus());

    QVariantMap options;
    QDBusReply<void> reply =
      iface.call("NotifyPointerAxisDiscrete",
                 QDBusObjectPath(m_sessionHandle),
                 options,
                 axis,
                 steps);

    if (!reply.isValid()) {
        qWarning() << "NotifyPointerAxisDiscrete failed:" << reply.error().message();
        return false;
    }

    return true;
}

bool WaylandPortalCaptureBackend::waitForFrameChange(const QImage& previous, int timeoutMs)
{
    const int sleepStepMs = 60;
    int waited = 0;

    while (waited < timeoutMs) {
        QImage current = latestFrame();

        if (!current.isNull() && !previous.isNull()) {
            if (current.size() == previous.size()) {
                if (current != previous) {
                    qDebug() << "Frame change detected after" << waited << "ms";
                    return true;
                }
            } else {
                qDebug() << "Frame size/content change detected after" << waited << "ms";
                return true;
            }
        }

        QThread::msleep(sleepStepMs);
        waited += sleepStepMs;
    }

    qDebug() << "Timeout waiting for frame change.";
    return false;
}

bool WaylandPortalCaptureBackend::scrollAndWaitChange(const QImage& previous, int steps)
{
    if (!scrollDown(steps)) {
        qDebug() << "scrollDown() failed.";
        return false;
    }

    QThread::msleep(150);

    waitForFrameChange(previous, 1200);
    return true;
}

bool WaylandPortalCaptureBackend::waitAfterExternalScroll(const QImage& previous, int timeoutMs)
{
    return waitForFrameChange(previous, timeoutMs);
}

bool WaylandPortalCaptureBackend::callRemoteDesktopNotifyPointerAxis(double dx, double dy)
{
    QDBusInterface iface(
      kPortalService,
      kRemoteDesktopPath,
      kRemoteDesktopInterface,
      QDBusConnection::sessionBus());

    QVariantMap options;

    QDBusReply<void> reply =
      iface.call("NotifyPointerAxis",
                 QDBusObjectPath(m_sessionHandle),
                 options,
                 dx,
                 dy);

    if (!reply.isValid()) {
        qWarning() << "NotifyPointerAxis failed:" << reply.error().message();
        return false;
    }

    return true;
}

bool WaylandPortalCaptureBackend::callRemoteDesktopNotifyPointerMotionAbsolute(uint stream, double x, double y)
{
    QDBusInterface iface(
      kPortalService,
      kRemoteDesktopPath,
      kRemoteDesktopInterface,
      QDBusConnection::sessionBus());

    QVariantMap options;

    QDBusReply<void> reply =
      iface.call("NotifyPointerMotionAbsolute",
                 QDBusObjectPath(m_sessionHandle),
                 options,
                 stream,
                 x,
                 y);

    if (!reply.isValid()) {
        qWarning() << "NotifyPointerMotionAbsolute failed:" << reply.error().message();
        return false;
    }

    return true;
}

bool WaylandPortalCaptureBackend::movePointerToSelectedRectCenter()
{
    if (!m_ready || m_streamNodeId == 0 || !m_selectedRect.isValid()) {
        return false;
    }

    QPoint center = m_selectedRect.center();

    qDebug() << "Moving pointer to center of selected rect:"
             << center
             << "stream =" << m_streamNodeId;

    return callRemoteDesktopNotifyPointerMotionAbsolute(
      m_streamNodeId,
      static_cast<double>(center.x()),
      static_cast<double>(center.y()));
}
#endif
