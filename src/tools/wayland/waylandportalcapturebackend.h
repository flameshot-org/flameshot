#pragma once

#include <QtGlobal>

#if defined(Q_OS_LINUX)

#include "portalrequesthelper.h"

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QRect>
#include <QString>

extern "C" {
#include <pipewire/stream.h>
}

struct pw_thread_loop;
struct pw_context;
struct pw_core;
struct pw_registry;
struct pw_stream;
struct spa_hook;
struct spa_pod;
struct spa_dict;

class WaylandPortalCaptureBackend : public QObject
{
    Q_OBJECT

public:
    explicit WaylandPortalCaptureBackend(QObject* parent = nullptr);
    ~WaylandPortalCaptureBackend() override;

    bool initialize(const QString& parentWindowId = QString());
    QImage latestFrame() const;
    bool scrollDown(int steps);
    void shutdown();

    bool isReady() const;
    QRect selectedRect() const;
    void setSelectedRect(const QRect& rect);

           // Callbacks de PipeWire / registry
    static void onRegistryGlobal(void* data,
                                 uint32_t id,
                                 uint32_t permissions,
                                 const char* type,
                                 uint32_t version,
                                 const struct spa_dict* props);

    static void onRegistryGlobalRemove(void* data, uint32_t id);

    static void onStreamStateChanged(void* data,
                                     enum pw_stream_state oldState,
                                     enum pw_stream_state state,
                                     const char* error);

    static void onStreamParamChanged(void* data,
                                     uint32_t id,
                                     const struct spa_pod* param);

    static void onStreamProcess(void* data);
    bool waitAfterExternalScroll(const QImage& previous, int timeoutMs = 1200);
    bool movePointerToSelectedRectCenter();

private:
    bool createRemoteDesktopSession();
    bool selectDevicesAndSources();
    bool startSession(const QString& parentWindowId);
    bool openPipeWireRemote();
    bool startPipeWireStream();
    void stopPipeWire();

    void handleRegistryGlobal(uint32_t id,
                              const char* type,
                              uint32_t version);
    void handleStreamProcess();

    bool callRemoteDesktopNotifyPointerAxis(double dx, double dy);
    bool callRemoteDesktopNotifyPointerAxisDiscrete(uint axis, int steps);

    void loadRestoreToken();
    void saveRestoreToken() const;
    void clearRestoreToken();

    bool waitForFirstFrame(int timeoutMs = 3000);
    bool hasValidFrame() const;
    bool waitForFrameChange(const QImage& previous, int timeoutMs = 1200);
    bool scrollAndWaitChange(const QImage& previous, int steps = 3);
    bool callRemoteDesktopNotifyPointerMotionAbsolute(uint stream, double x, double y);

private:
    QString m_sessionHandle;
    QString m_restoreToken;
    QString m_parentWindowId;

    uint32_t m_streamNodeId = 0;
    QRect m_selectedRect;

    int m_pipewireFd = -1;
    bool m_ready = false;

    mutable QMutex m_frameMutex;
    QImage m_latestFrame;

    pw_thread_loop* m_pwLoop = nullptr;
    pw_context* m_pwContext = nullptr;
    pw_core* m_pwCore = nullptr;
    pw_registry* m_pwRegistry = nullptr;
    pw_stream* m_pwStream = nullptr;

    spa_hook m_registryHook {};
    spa_hook m_streamHook {};

};
#endif
