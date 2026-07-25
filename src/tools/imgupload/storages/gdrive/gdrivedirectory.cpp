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
#include <QRegularExpression>
#include <QSet>
#include <QTimer>
#include <QUrlQuery>

namespace {
const char* kPeopleSearchEndpoint =
  "https://people.googleapis.com/v1/people:searchDirectoryPeople";
// Cloud Identity rather than the Admin SDK Directory API: the Directory API's
// Group resource requires the caller to hold an admin role, which ordinary
// users do not (KD4). This endpoint's direct-membership search needs no admin
// role and no customer identifier -- which is also why it covers direct
// memberships only (KD5).
const char* kDirectGroupsEndpoint = "https://cloudidentity.googleapis.com/v1/"
                                    "groups/-/memberships:searchDirectGroups";

constexpr int kRequestTimeoutMs = 15 * 1000;
// Long enough to swallow the middle of a word, short enough to feel live.
constexpr int kDebounceMs = 250;
// One letter matches most of an organization; the request would be noise. This
// dialog opens after every capture, so request volume is worth bounding (KTD7).
constexpr int kMinimumPrefixLength = 2;
constexpr int kMaxResults = 10;
// One page of memberships is plenty: these are the user's own direct groups.
constexpr int kMaxGroups = 200;

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

/**
 * Does `text` start with `prefix`, at its beginning or at a word boundary?
 *
 * The word-boundary half mirrors how the directory's own people search behaves,
 * so a group behaves like a person: typing "platform" finds "Core Platform
 * Team" and "core-platform@example.com", not just names that begin with it.
 */
bool matchesPrefix(const QString& text, const QString& prefix)
{
    static const QRegularExpression wordBreak(QStringLiteral("[\\s._@-]+"));

    if (text.startsWith(prefix, Qt::CaseInsensitive)) {
        return true;
    }
    const QStringList words = text.split(wordBreak, Qt::SkipEmptyParts);
    for (const QString& word : words) {
        if (word.startsWith(prefix, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
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
  , m_groupsLoaded(false)
  , m_groupsFetching(false)
  , m_groupsReply(nullptr)
{
    m_debounce->setSingleShot(true);
    m_debounce->setInterval(kDebounceMs);
    connect(
      m_debounce, &QTimer::timeout, this, [this]() { searchPeople(m_prefix); });

    // A cached membership list outliving its account would offer the previous
    // user's teams after a switch (KTD5).
    connect(GDriveOAuth::instance(),
            &GDriveOAuth::accountChanged,
            this,
            &GDriveDirectory::dropGroupCache);
}

void GDriveDirectory::requestSuggestions(const QString& prefix)
{
    m_prefix = prefix;
    m_people.clear();
    // Abandon the previous search before starting another, so an older reply
    // can never land on top of a newer one (KTD7).
    abortPeopleSearch();
    m_debounce->stop();

    if (prefix.size() < kMinimumPrefixLength) {
        emit suggestionsReady(prefix, QList<RecipientSuggestion>());
        return;
    }

    ensureGroups();
    // Groups resolve without a round trip, so they can be offered immediately;
    // the people reply extends the list when it lands. Groups stay first for
    // exactly that reason -- rows must not reorder under the cursor (KTD6).
    emitCurrent();
    m_debounce->start();
}

void GDriveDirectory::emitCurrent()
{
    QList<RecipientSuggestion> combined = matchGroups(m_prefix);
    combined.append(m_people);
    emit suggestionsReady(m_prefix, combined);
}

void GDriveDirectory::searchPeople(const QString& prefix)
{
    const QString account = GDriveOAuth::instance()->accountEmail();
    // The silent token path only: a lookup that could open a consent window
    // would put a browser tab in front of a user who is mid-share (KTD1).
    GDriveOAuth::instance()->requestAccessTokenSilently(
      [this, prefix, account](const QString& token) {
          if (prefix != m_prefix) {
              return; // the user typed on while the token was being acquired
          }
          if (accountChangedSince(account)) {
              // Never search one organization's directory on behalf of another.
              emit suggestionsReady(prefix, QList<RecipientSuggestion>());
              return;
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
                if (reply->error() == QNetworkReply::NoError) {
                    m_people = parsePeople(reply->readAll());
                }
                // A refused people call leaves whatever groups matched in
                // place: one source failing must not take the other down
                // (KTD2).
                emitCurrent();
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

void GDriveDirectory::ensureGroups()
{
    if (m_groupsLoaded || m_groupsFetching) {
        return;
    }
    const QString member = GDriveOAuth::instance()->accountEmail();
    if (member.isEmpty()) {
        // No known account to ask about. Left unloaded rather than marked
        // empty: the address arrives with the first consent, and the next
        // keystroke after that should find groups.
        return;
    }

    m_groupsFetching = true;
    GDriveOAuth::instance()->requestAccessTokenSilently(
      [this, member](const QString& token) {
          if (token.isEmpty()) {
              // Transient: no usable grant yet. Try again on a later keystroke
              // rather than settling on an empty membership list for the
              // session.
              m_groupsFetching = false;
              return;
          }
          if (accountChangedSince(member)) {
              // Disconnected or switched while the token was being acquired.
              // Issuing this call now would fill the cache with the previous
              // user's teams, which is exactly what dropping it on an account
              // change exists to prevent (KTD5).
              m_groupsFetching = false;
              return;
          }

          QUrl url{ QString::fromLatin1(kDirectGroupsEndpoint) };
          QUrlQuery query;
          // Filtered by member only. Adding a label clause -- which the
          // organization-wide search requires -- makes this endpoint answer
          // 400.
          query.addQueryItem(
            QStringLiteral("query"),
            QStringLiteral("member_key_id == '%1'").arg(member));
          query.addQueryItem(QStringLiteral("pageSize"),
                             QString::number(kMaxGroups));
          url.setQuery(query);

          QNetworkRequest request{ url };
          request.setRawHeader("Authorization",
                               QStringLiteral("Bearer %1").arg(token).toUtf8());
          request.setTransferTimeout(kRequestTimeoutMs);

          m_groupsReply = m_net->get(request);
          QNetworkReply* reply = m_groupsReply;
          connect(reply, &QNetworkReply::finished, this, [this, reply]() {
              reply->deleteLater();
              m_groupsFetching = false;
              if (m_groupsReply != reply) {
                  return; // dropped by an account change while in flight
              }
              m_groupsReply = nullptr;
              // Loaded either way: a tenant that refuses the group call refuses
              // it every time, so one attempt per process is enough and people
              // suggestions carry on unaffected.
              m_groupsLoaded = true;
              if (reply->error() == QNetworkReply::NoError) {
                  m_groups = parseGroups(reply->readAll());
              }
              if (m_prefix.size() >= kMinimumPrefixLength) {
                  emitCurrent();
              }
          });
      });
}

void GDriveDirectory::dropGroupCache()
{
    if (m_groupsReply != nullptr) {
        // Clear the handle first: abort() delivers finished() straight away,
        // and the handler must recognize this reply as no longer wanted.
        QNetworkReply* reply = m_groupsReply;
        m_groupsReply = nullptr;
        reply->abort();
    }
    m_groups.clear();
    m_groupsLoaded = false;
    m_groupsFetching = false;
}

bool GDriveDirectory::accountChangedSince(const QString& account) const
{
    return account != GDriveOAuth::instance()->accountEmail();
}

QList<RecipientSuggestion> GDriveDirectory::matchGroups(
  const QString& prefix) const
{
    QList<RecipientSuggestion> matches;
    for (const RecipientSuggestion& group : m_groups) {
        if (matchesPrefix(group.displayName, prefix) ||
            matchesPrefix(group.address, prefix)) {
            matches.append(group);
        }
        if (matches.size() >= kMaxResults) {
            break;
        }
    }
    return matches;
}

QList<RecipientSuggestion> GDriveDirectory::parseGroups(
  const QByteArray& payload)
{
    const QJsonArray memberships = QJsonDocument::fromJson(payload)
                                     .object()
                                     .value(QStringLiteral("memberships"))
                                     .toArray();

    QList<RecipientSuggestion> groups;
    QSet<QString> seen;
    for (const QJsonValue& value : memberships) {
        const QJsonObject membership = value.toObject();
        const QString address = membership.value(QStringLiteral("groupKey"))
                                  .toObject()
                                  .value(QStringLiteral("id"))
                                  .toString();
        if (address.isEmpty()) {
            continue;
        }
        const QString key = address.toLower();
        if (seen.contains(key)) {
            continue;
        }
        seen.insert(key);

        RecipientSuggestion group;
        // The description this response also carries is dropped: it adds width
        // without helping identify the right recipient (KTD9).
        group.displayName =
          membership.value(QStringLiteral("displayName")).toString();
        group.address = address;
        group.isGroup = true;
        groups.append(group);
    }
    return groups;
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
