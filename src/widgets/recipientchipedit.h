// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Flameshot Contributors

#pragma once

#include <QFrame>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>

class QCompleter;
class QLineEdit;
class QStringListModel;

/**
 * One suggested recipient: a person or a group, as some directory reports it.
 *
 * Name and address only. A group's description is deliberately absent -- it
 * adds width without helping identify the right recipient (KTD9).
 */
struct RecipientSuggestion
{
    QString displayName;
    QString address;
    bool isGroup = false;
};

/**
 * Where RecipientChipEdit gets its suggestions. Backend-neutral on purpose: the
 * widget compiles wherever the upload dialog does, including builds with no
 * Google Drive support, so it must not name a backend (KTD4).
 */
class RecipientSuggestionSource : public QObject
{
    Q_OBJECT
public:
    explicit RecipientSuggestionSource(QObject* parent = nullptr);
    ~RecipientSuggestionSource() override = default;

    /**
     * Ask for recipients matching `prefix`.
     *
     * Implementations answer through suggestionsReady -- possibly more than
     * once for one prefix (a locally-matched batch first, a network result
     * after), and possibly not at all once a newer prefix has superseded this
     * one. This call must not block, and an unavailable directory is reported
     * as an empty result rather than an error: suggestions assist a share, they
     * never gate one (KD3).
     */
    virtual void requestSuggestions(const QString& prefix) = 0;

signals:
    void suggestionsReady(const QString& prefix,
                          const QList<RecipientSuggestion>& suggestions);
};

/**
 * A recipient field where each confirmed recipient is a removable chip and the
 * user keeps typing after the last one.
 *
 * Replaces a plain comma-separated line edit so that a resolved person stays
 * visually distinct from an address that matched nobody (KD2): a resolved chip
 * carries the directory's name alongside the address -- always both, since
 * distinct people in one organization can share a display name (KD7) -- while
 * an unresolved entry is marked and still submitted unchanged (R6).
 *
 * Suggestions come from an injected RecipientSuggestionSource, which may be
 * absent. With no source, or with a source that never finds anything, the field
 * still accepts typed addresses exactly as the old line edit did (R7, R11).
 */
class RecipientChipEdit : public QFrame
{
    Q_OBJECT
public:
    explicit RecipientChipEdit(QWidget* parent = nullptr);

    /** Attach a directory to suggest from. Not owned; null means no
     * suggestions. */
    void setSuggestionSource(RecipientSuggestionSource* source);

    /** Committed addresses, in entry order, without duplicates. */
    QStringList addresses() const;

    /**
     * Commit whatever is typed but not yet a chip, and return the text that
     * could not become one. A caller about to act on addresses() can then say
     * so rather than dropping the entry silently, which is how a typo used to
     * reach the upload unnoticed.
     */
    QString commitPendingText();

    /** Split `text` on separators and keep the parts that are valid addresses.
     */
    static QStringList parseAddresses(const QString& text);
    static bool isValidAddress(const QString& candidate);

signals:
    void recipientsChanged();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    struct Chip
    {
        QString displayName;
        QString address;
        bool resolved;
        QFrame* widget;
    };

    void onTextEdited(const QString& text);
    void onSuggestionsReady(const QString& prefix,
                            const QList<RecipientSuggestion>& suggestions);
    void onSuggestionActivated(const QString& label);

    void addRecipient(const QString& displayName,
                      const QString& address,
                      bool resolved);
    void commitAddress(const QString& address);
    void removeChip(QFrame* widget);
    void removeLastChip();
    QFrame* createChip(const QString& displayName,
                       const QString& address,
                       bool resolved);

    void setInputText(const QString& text);
    void hidePopup();

    static QString suggestionLabel(const RecipientSuggestion& suggestion);

    RecipientSuggestionSource* m_source; // not owned
    QLineEdit* m_input;
    QCompleter* m_completer;
    QStringListModel* m_model;

    // The suggestions currently offered, parallel to the completer's model, so
    // an activated row maps back to the address the directory reported rather
    // than to the label the user saw.
    QList<RecipientSuggestion> m_suggestions;
    QVector<Chip> m_chips;
};
