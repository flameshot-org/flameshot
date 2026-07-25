// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Flameshot Contributors

#include "gdrivedirectory.h"
#include "gdriveoauth.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSet>
#include <QTimer>
#include <QUrlQuery>

namespace {
const char* kPeopleSearchEndpoint =
  "https://people.googleapis.com/v1/people:searchDirectoryPeople";

constexpr int kRequestTimeoutMs = 15 * 1000;
// Long enough to swallow the middle of a word, short enough to feel live.
constexpr int kDebounceMs = 250;
// One letter matches most of an organization; the request would be noise. This
// dialog opens after every capture, so request volume is worth bounding (KTD7).
constexpr int kMinimumPrefixLength = 2;
constexpr int kMaxResults = 10;

/**
 * The value the directory marks primary, or the first one it reports.
 *
 * Which address a picked suggestion contributes rests on this marker: an
 * alternate address is how a colleague was found, not how they should be
 * addressed (R3).
 */
QString primaryValue(const QJsonObject& person,
                     const QString& field,
                     const QString& key)
{
    const QJsonArray entries = person.value(field).toArray();
    QString fallback;
    for (const QJsonValue& value : entries) {
        const QJsonObject entry = value.toObject();
        const QString text = entry.value(key).toString();
        if (text.isEmpty()) {
            continue;
        }
        if (entry.value(QStringLiteral("metadata"))
              .toObject()
              .value(QStringLiteral("primary"))
              .toBool()) {
            return text;
        }
        if (fallback.isEmpty()) {
            fallback = text;
        }
    }
    return fallback;
}
}

GDriveDirectory* GDriveDirectory::instance()
{
    static GDriveDirectory* self = new GDriveDirectory(qApp);
    return self;
}

GDriveDirectory::GDriveDirectory(QObject* parent)
  : RecipientSuggestionSource(parent)
  , m_net(new QNetworkAccessManager(this))
  , m_debounce(new QTimer(this))
  , m_peopleReply(nullptr)
{
    m_debounce->setSingleShot(true);
    m_debounce->setInterval(kDebounceMs);
    connect(
      m_debounce, &QTimer::timeout, this, [this]() { searchPeople(m_prefix); });
}

void GDriveDirectory::requestSuggestions(const QString& prefix)
{
    m_prefix = prefix;
    // Abandon the previous search before starting another, so an older reply
    // can never land on top of a newer one (KTD7).
    abortPeopleSearch();
    m_debounce->stop();

    if (prefix.size() < kMinimumPrefixLength) {
        emit suggestionsReady(prefix, QList<RecipientSuggestion>());
        return;
    }
    m_debounce->start();
}

void GDriveDirectory::searchPeople(const QString& prefix)
{
    // The silent token path only: a lookup that could open a consent window
    // would put a browser tab in front of a user who is mid-share (KTD1).
    GDriveOAuth::instance()->requestAccessTokenSilently(
      [this, prefix](const QString& token) {
          if (prefix != m_prefix) {
              return; // the user typed on while the token was being acquired
          }
          if (token.isEmpty()) {
              emit suggestionsReady(prefix, QList<RecipientSuggestion>());
              return;
          }

          QUrl url{ QString::fromLatin1(kPeopleSearchEndpoint) };
          QUrlQuery query;
          // The endpoint prefix-matches across person fields, so one query
          // covers both "starts typing a surname" and "starts typing an
          // address", including alternates (R1, R3, AE2, AE5).
          query.addQueryItem(QStringLiteral("query"), prefix);
          query.addQueryItem(QStringLiteral("readMask"),
                             QStringLiteral("names,emailAddresses"));
          query.addQueryItem(
            QStringLiteral("sources"),
            QStringLiteral("DIRECTORY_SOURCE_TYPE_DOMAIN_PROFILE"));
          query.addQueryItem(
            QStringLiteral("sources"),
            QStringLiteral("DIRECTORY_SOURCE_TYPE_DOMAIN_CONTACT"));
          query.addQueryItem(QStringLiteral("pageSize"),
                             QString::number(kMaxResults));
          url.setQuery(query);

          QNetworkRequest request{ url };
          request.setRawHeader("Authorization",
                               QStringLiteral("Bearer %1").arg(token).toUtf8());
          request.setTransferTimeout(kRequestTimeoutMs);

          abortPeopleSearch();
          m_peopleReply = m_net->get(request);
          QNetworkReply* reply = m_peopleReply;
          connect(
            reply, &QNetworkReply::finished, this, [this, reply, prefix]() {
                reply->deleteLater();
                if (m_peopleReply != reply) {
                    // Abandoned in favor of a newer search: stale by
                    // construction.
                    return;
                }
                m_peopleReply = nullptr;
                if (prefix != m_prefix) {
                    return;
                }
                if (reply->error() != QNetworkReply::NoError) {
                    emit suggestionsReady(prefix, QList<RecipientSuggestion>());
                    return;
                }
                emit suggestionsReady(prefix, parsePeople(reply->readAll()));
            });
      });
}

void GDriveDirectory::abortPeopleSearch()
{
    if (m_peopleReply == nullptr) {
        return;
    }
    // Clear the handle first: abort() delivers finished() straight away, and
    // the handler recognizes an abandoned reply by no longer being the current
    // one.
    QNetworkReply* reply = m_peopleReply;
    m_peopleReply = nullptr;
    reply->abort();
}

QList<RecipientSuggestion> GDriveDirectory::parsePeople(
  const QByteArray& payload)
{
    const QJsonArray people = QJsonDocument::fromJson(payload)
                                .object()
                                .value(QStringLiteral("people"))
                                .toArray();

    QList<RecipientSuggestion> suggestions;
    QSet<QString> seen;
    for (const QJsonValue& value : people) {
        const QJsonObject person = value.toObject();
        const QString address = primaryValue(
          person, QStringLiteral("emailAddresses"), QStringLiteral("value"));
        if (address.isEmpty()) {
            continue; // nothing to share with
        }
        // One directory entry is one row, even when a name and an alias both
        // matched the prefix (KTD6).
        const QString key = address.toLower();
        if (seen.contains(key)) {
            continue;
        }
        seen.insert(key);

        RecipientSuggestion suggestion;
        suggestion.displayName = primaryValue(
          person, QStringLiteral("names"), QStringLiteral("displayName"));
        suggestion.address = address;
        suggestion.isGroup = false;
        suggestions.append(suggestion);
    }
    return suggestions;
}
