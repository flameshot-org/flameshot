// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Flameshot Contributors

#pragma once

#include "widgets/recipientchipedit.h"

#include <QList>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

/**
 * Recipient suggestions read live from the organization's Google directory.
 *
 * Nothing is written to disk: results live in memory for the life of the
 * process and are gone at exit, so no colleague's name, address, or group ever
 * joins the refresh token in the configuration file (R4, KD1).
 *
 * Every failure is one path. A declined consent, a tenant that restricts
 * directory visibility, a revoked grant, and a machine with no network all
 * surface as "no suggestions" -- never as an error dialog and never as a
 * blocked upload (KTD2, R11). Availability is discovered by attempting the call
 * rather than by recording which scopes Google granted, because the granted set
 * is not stored and a second marker would be one more thing to keep true.
 *
 * Directory content and typed prefixes are never logged. The OAuth service
 * already holds that line for codes and tokens; colleagues' names and whatever
 * the user types are the same class of data (KTD11).
 */
class GDriveDirectory : public RecipientSuggestionSource
{
    Q_OBJECT
public:
    /** Application-lifetime service, like the OAuth one it borrows tokens from.
     */
    static GDriveDirectory* instance();

    void requestSuggestions(const QString& prefix) override;

private:
    explicit GDriveDirectory(QObject* parent = nullptr);

    void searchPeople(const QString& prefix);
    void abortPeopleSearch();

    static QList<RecipientSuggestion> parsePeople(const QByteArray& payload);

    QNetworkAccessManager* m_net;
    QTimer* m_debounce;

    // The people search in flight, if any. Held so a superseded one can be
    // abandoned before a newer one is issued.
    QNetworkReply* m_peopleReply;

    // The prefix the user is on now. Anything answering for a different prefix
    // is stale and dropped.
    QString m_prefix;
};
