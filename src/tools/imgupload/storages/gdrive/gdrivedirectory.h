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

    /**
     * Fetch the signed-in user's direct group memberships once per process.
     *
     * The group call filters by member rather than by typed text, so it returns
     * the whole membership list in one response and matching happens locally --
     * only the people search is a live per-keystroke query (KD8).
     *
     * A dialog is constructed per capture, so a dialog-scoped cache would
     * re-fetch on every screenshot. The list therefore lives here, beside the
     * application-lifetime OAuth service, and is dropped when the account
     * changes so the previous user's teams can never be offered to the next one
     * (KTD5).
     */
    void ensureGroups();
    void dropGroupCache();
    QList<RecipientSuggestion> matchGroups(const QString& prefix) const;

    /** Emit groups then people for the current prefix, whichever has arrived.
     */
    void emitCurrent();

    /**
     * Has the connected account changed since a lookup was started for
     * `account`?
     *
     * A token is acquired asynchronously, so a disconnect or an account switch
     * can land between starting a lookup and issuing it. Without this check the
     * lookup would go out under the previous account and its results -- another
     * organization's directory, another user's groups -- would be offered to
     * whoever is connected now (KTD5). An empty address compares equal to an
     * empty one, so an account whose sign-in scopes were declined still gets
     * suggestions.
     */
    bool accountChangedSince(const QString& account) const;

    static QList<RecipientSuggestion> parsePeople(const QByteArray& payload);
    static QList<RecipientSuggestion> parseGroups(const QByteArray& payload);

    QNetworkAccessManager* m_net;
    QTimer* m_debounce;

    // The people search in flight, if any. Held so a superseded one can be
    // abandoned before a newer one is issued.
    QNetworkReply* m_peopleReply;

    // The prefix the user is on now. Anything answering for a different prefix
    // is stale and dropped.
    QString m_prefix;

    // The people result for m_prefix, kept so a group list arriving afterwards
    // extends the suggestions rather than replacing them.
    QList<RecipientSuggestion> m_people;

    // Direct group memberships, matched locally. `m_groupsLoaded` covers a
    // refusal too: a tenant that declines the call declines it every time, so
    // one attempt per process is enough.
    QList<RecipientSuggestion> m_groups;
    bool m_groupsLoaded;
    bool m_groupsFetching;
    QNetworkReply* m_groupsReply;
};
