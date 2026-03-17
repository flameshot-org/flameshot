#pragma once

#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QMap>
#include <QObject>
#include <QVariant>
#include <optional>

struct PortalRequestResponse
{
    uint responseCode = 2;
    QVariantMap results;
};

struct PortalStream
{
    uint nodeId = 0;
    QVariantMap properties;
};

Q_DECLARE_METATYPE(PortalRequestResponse)
Q_DECLARE_METATYPE(PortalStream)
Q_DECLARE_METATYPE(QList<PortalStream>)

using VariantMapMap = QMap<QString, QVariantMap>;
Q_DECLARE_METATYPE(VariantMapMap)

const QDBusArgument& operator<<(QDBusArgument& argument, const PortalStream& stream);
const QDBusArgument& operator>>(const QDBusArgument& argument, PortalStream& stream);

class PortalRequestHelper : public QObject
{
    Q_OBJECT

public:
    explicit PortalRequestHelper(QObject* parent = nullptr);

    static QString makeHandleToken(const QString& prefix);
    static QVariantMap makeOptionsWithToken(const QString& tokenKey, const QString& prefix);

    std::optional<PortalRequestResponse> waitForResponse(
      const QString& requestPath,
      int timeoutMs = 120000);

private slots:
    void onResponse(uint response, const QVariantMap& results);

private:
    bool m_done = false;
    PortalRequestResponse m_response;
};
