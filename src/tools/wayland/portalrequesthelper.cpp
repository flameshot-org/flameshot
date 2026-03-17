#include "portalrequesthelper.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QEventLoop>
#include <QRandomGenerator>
#include <QTimer>

static const char* kPortalService = "org.freedesktop.portal.Desktop";
static const char* kRequestInterface = "org.freedesktop.portal.Request";

const QDBusArgument& operator<<(QDBusArgument& argument, const PortalStream& stream)
{
    argument.beginStructure();
    argument << stream.nodeId << stream.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, PortalStream& stream)
{
    argument.beginStructure();
    argument >> stream.nodeId >> stream.properties;
    argument.endStructure();
    return argument;
}

PortalRequestHelper::PortalRequestHelper(QObject* parent)
  : QObject(parent)
{
}

QString PortalRequestHelper::makeHandleToken(const QString& prefix)
{
    const quint64 r = QRandomGenerator::global()->generate64();
    return QString("%1_%2").arg(prefix).arg(r, 0, 16);
}

QVariantMap PortalRequestHelper::makeOptionsWithToken(const QString& tokenKey,
                                                      const QString& prefix)
{
    QVariantMap opts;
    opts.insert(tokenKey, makeHandleToken(prefix));
    return opts;
}

std::optional<PortalRequestResponse> PortalRequestHelper::waitForResponse(
  const QString& requestPath,
  int timeoutMs)
{
    m_done = false;
    m_response = {};

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    const bool connected = QDBusConnection::sessionBus().connect(
      kPortalService,
      requestPath,
      kRequestInterface,
      "Response",
      this,
      SLOT(onResponse(uint,QVariantMap)));

    if (!connected) {
        return std::nullopt;
    }

    connect(this, &QObject::destroyed, &loop, &QEventLoop::quit);
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    timeout.start(timeoutMs);

    while (!m_done && timeout.isActive()) {
        loop.processEvents(QEventLoop::AllEvents, 50);
    }

    QDBusConnection::sessionBus().disconnect(
      kPortalService,
      requestPath,
      kRequestInterface,
      "Response",
      this,
      SLOT(onResponse(uint,QVariantMap)));

    if (!m_done) {
        return std::nullopt;
    }

    return m_response;
}

void PortalRequestHelper::onResponse(uint response, const QVariantMap& results)
{
    m_response.responseCode = response;
    m_response.results = results;
    m_done = true;
}
