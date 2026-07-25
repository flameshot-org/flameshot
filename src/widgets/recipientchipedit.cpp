// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Flameshot Contributors

#include "recipientchipedit.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRegularExpression>
#include <QStringListModel>
#include <QToolButton>

namespace {
/**
 * Only a comma or a semicolon commits a typed entry (KTD8), preserving the
 * separators the old field already accepted so pasting a list keeps working.
 * Whitespace deliberately does not commit: the same input is the search box,
 * and "type a colleague's name" means spaces arrive mid-prefix.
 */
bool isCommitSeparator(QChar character)
{
    return character == QLatin1Char(',') || character == QLatin1Char(';');
}

// Border for an entry that matched nobody. A fixed accent rather than a palette
// role because no palette role means "suspect", and it reads on light and dark
// themes alike. The warning never rests on color alone: the chip is also marked
// and carries a tooltip.
const QColor& unresolvedBorder()
{
    static const QColor color(0xC0, 0x39, 0x2B);
    return color;
}

constexpr int kInputMinimumWidth = 140;

/**
 * A layout that fills each line and wraps, stretching its last item to the end
 * of the line it lands on.
 *
 * Chips and the typing area share one run so that typing continues after the
 * last chip (R5) and a long recipient list grows downward instead of clipping.
 * Qt ships no wrapping layout; this is the minimum that serves the field, and
 * it stays private to this widget.
 */
class ChipFlowLayout : public QLayout
{
public:
    explicit ChipFlowLayout(QWidget* parent)
      : QLayout(parent)
    {
        setContentsMargins(4, 4, 4, 4);
        setSpacing(4);
    }

    ~ChipFlowLayout() override
    {
        while (QLayoutItem* item = takeAt(0)) {
            delete item;
        }
    }

    void addItem(QLayoutItem* item) override { m_items.append(item); }
    int count() const override { return static_cast<int>(m_items.size()); }
    QLayoutItem* itemAt(int index) const override
    {
        return m_items.value(index);
    }

    QLayoutItem* takeAt(int index) override
    {
        if (index < 0 || index >= m_items.size()) {
            return nullptr;
        }
        return m_items.takeAt(index);
    }

    Qt::Orientations expandingDirections() const override { return {}; }
    bool hasHeightForWidth() const override { return true; }

    int heightForWidth(int width) const override
    {
        return doLayout(QRect(0, 0, width, 0), true);
    }

    void setGeometry(const QRect& rect) override
    {
        QLayout::setGeometry(rect);
        doLayout(rect, false);
    }

    QSize sizeHint() const override { return minimumSize(); }

    QSize minimumSize() const override
    {
        QSize size;
        for (QLayoutItem* item : m_items) {
            size = size.expandedTo(item->minimumSize());
        }
        const QMargins margins = contentsMargins();
        return size + QSize(margins.left() + margins.right(),
                            margins.top() + margins.bottom());
    }

private:
    int doLayout(const QRect& rect, bool testOnly) const
    {
        const QMargins margins = contentsMargins();
        const QRect line = rect.adjusted(
          margins.left(), margins.top(), -margins.right(), -margins.bottom());
        int x = line.x();
        int y = line.y();
        int lineHeight = 0;

        for (int i = 0; i < m_items.size(); ++i) {
            QLayoutItem* item = m_items.at(i);
            const QSize hint = item->sizeHint();
            // The typing area is always last and takes the rest of its line, so
            // the field looks and behaves like the text field it replaces.
            const bool stretch = i == m_items.size() - 1;
            const int needed =
              stretch ? qMax(item->minimumSize().width(), hint.width())
                      : hint.width();

            if (x + needed > line.right() + 1 && lineHeight > 0) {
                x = line.x();
                y += lineHeight + spacing();
                lineHeight = 0;
            }

            const int width =
              stretch ? qMax(needed, line.right() - x + 1) : hint.width();
            if (!testOnly) {
                item->setGeometry(QRect(x, y, width, hint.height()));
            }
            x += width + spacing();
            lineHeight = qMax(lineHeight, hint.height());
        }
        return y + lineHeight - rect.y() + margins.bottom();
    }

    QList<QLayoutItem*> m_items;
};
}

RecipientSuggestionSource::RecipientSuggestionSource(QObject* parent)
  : QObject(parent)
{}

RecipientChipEdit::RecipientChipEdit(QWidget* parent)
  : QFrame(parent)
  , m_source(nullptr)
  , m_input(new QLineEdit(this))
  , m_completer(new QCompleter(this))
  , m_model(new QStringListModel(this))
{
    // Look like the field this replaces, so the dialog reads unchanged.
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    // The chip run wraps, so the field's height depends on its width -- and a
    // parent layout only asks for that when the size policy says so. Without
    // this the field keeps its one-line height and every chip past the first
    // line is clipped.
    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);
    setFocusProxy(m_input);

    auto* flow = new ChipFlowLayout(this);
    m_input->setFrame(false);
    m_input->setMinimumWidth(kInputMinimumWidth);
    m_input->setPlaceholderText(tr("Type an email address"));
    flow->addWidget(m_input);

    m_completer->setModel(m_model);
    // The source has already prefix-filtered, so the completer must not filter
    // again -- unfiltered popup mode shows exactly the rows it was handed.
    m_completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    // setWidget() rather than QLineEdit::setCompleter(): the completer must own
    // popup navigation without inserting its own text into the input, since a
    // picked row contributes the directory's address and not the label shown.
    m_completer->setWidget(m_input);
    connect(m_completer,
            QOverload<const QString&>::of(&QCompleter::activated),
            this,
            &RecipientChipEdit::onSuggestionActivated);

    connect(
      m_input, &QLineEdit::textEdited, this, &RecipientChipEdit::onTextEdited);
    m_input->installEventFilter(this);
}

void RecipientChipEdit::setSuggestionSource(RecipientSuggestionSource* source)
{
    if (m_source == source) {
        return;
    }
    if (m_source != nullptr) {
        disconnect(m_source, nullptr, this, nullptr);
    }
    m_source = source;
    if (m_source != nullptr) {
        connect(m_source,
                &RecipientSuggestionSource::suggestionsReady,
                this,
                &RecipientChipEdit::onSuggestionsReady);
    }
    m_input->setPlaceholderText(m_source != nullptr
                                  ? tr("Type a name, address, or group")
                                  : tr("Type an email address"));
}

QStringList RecipientChipEdit::addresses() const
{
    QStringList result;
    result.reserve(m_chips.size());
    for (const Chip& chip : m_chips) {
        result.append(chip.address);
    }
    return result;
}

QString RecipientChipEdit::commitPendingText()
{
    // Take the path a typed separator takes: a trailing separator makes the
    // whole pending text one complete entry. Anything that fails validation is
    // left in the input untouched, and reported back.
    onTextEdited(m_input->text() + QLatin1Char(','));
    return m_input->text().trimmed();
}

QStringList RecipientChipEdit::parseAddresses(const QString& text)
{
    static const QRegularExpression separator(QStringLiteral("[,;\\s]+"));

    QStringList valid;
    const QStringList parts = text.split(separator, Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        const QString candidate = part.trimmed();
        if (isValidAddress(candidate)) {
            valid.append(candidate);
        }
    }
    return valid;
}

bool RecipientChipEdit::isValidAddress(const QString& candidate)
{
    static const QRegularExpression emailPattern(
      QStringLiteral("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$"));

    return !candidate.isEmpty() && emailPattern.match(candidate).hasMatch();
}

bool RecipientChipEdit::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != m_input || event->type() != QEvent::KeyPress) {
        return QFrame::eventFilter(watched, event);
    }

    // While the popup is up it owns the arrow keys, Enter, and Escape: Enter
    // must pick the highlighted suggestion, not commit the raw prefix, and
    // Escape must close the popup rather than the dialog.
    if (m_completer->popup() != nullptr && m_completer->popup()->isVisible()) {
        return QFrame::eventFilter(watched, event);
    }

    auto* keyEvent = static_cast<QKeyEvent*>(event);
    switch (keyEvent->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Tab:
            // With text pending, these commit it (KTD8) instead of accepting
            // the dialog or moving focus. With the input empty they fall
            // through, so Enter still confirms the upload -- a lookup in flight
            // never stands between the user and the button (R8).
            if (!m_input->text().trimmed().isEmpty()) {
                commitPendingText();
                return true;
            }
            break;
        case Qt::Key_Backspace:
            if (m_input->text().isEmpty() && !m_chips.isEmpty()) {
                removeLastChip();
                return true;
            }
            break;
        default:
            break;
    }
    return QFrame::eventFilter(watched, event);
}

void RecipientChipEdit::onTextEdited(const QString& text)
{
    int position = -1;
    for (int i = 0; i < text.size() && position < 0; ++i) {
        if (isCommitSeparator(text.at(i))) {
            position = i;
        }
    }

    if (position < 0) {
        const QString prefix = text.trimmed();
        if (m_source != nullptr) {
            m_source->requestSuggestions(prefix);
        } else {
            hidePopup();
        }
        return;
    }

    const QString head = text.left(position).trimmed();
    const QString tail = text.mid(position + 1);
    if (!head.isEmpty() && !isValidAddress(head)) {
        // Leave it exactly as typed. Nothing is dropped and nothing is
        // invented: silently discarding a malformed address is how a mistyped
        // recipient used to reach the upload unnoticed.
        return;
    }
    if (!head.isEmpty()) {
        commitAddress(head);
    }
    setInputText(tail);
    // A pasted list arrives as one edit; keep consuming it entry by entry.
    onTextEdited(tail);
}

void RecipientChipEdit::onSuggestionsReady(
  const QString& prefix,
  const QList<RecipientSuggestion>& suggestions)
{
    // A reply for a prefix the user has already moved past must never replace
    // what is on screen now.
    if (prefix != m_input->text().trimmed()) {
        return;
    }

    m_suggestions = suggestions;
    QStringList labels;
    labels.reserve(suggestions.size());
    for (const RecipientSuggestion& suggestion : suggestions) {
        labels.append(suggestionLabel(suggestion));
    }
    m_model->setStringList(labels);

    if (labels.isEmpty()) {
        hidePopup();
        return;
    }
    m_completer->complete();
}

void RecipientChipEdit::onSuggestionActivated(const QString& label)
{
    for (const RecipientSuggestion& suggestion : m_suggestions) {
        if (suggestionLabel(suggestion) == label) {
            // The address the directory marked canonical, not the alias that
            // may have been typed to find it (R3).
            addRecipient(suggestion.displayName, suggestion.address, true);
            setInputText(QString());
            hidePopup();
            return;
        }
    }
}

void RecipientChipEdit::commitAddress(const QString& address)
{
    // An address typed in full that happens to be on offer is still a resolved
    // recipient: the user found the same person the directory did.
    for (const RecipientSuggestion& suggestion : m_suggestions) {
        if (suggestion.address.compare(address, Qt::CaseInsensitive) == 0) {
            addRecipient(suggestion.displayName, suggestion.address, true);
            return;
        }
    }
    addRecipient(QString(), address, false);
}

void RecipientChipEdit::addRecipient(const QString& displayName,
                                     const QString& address,
                                     bool resolved)
{
    for (const Chip& chip : m_chips) {
        if (chip.address.compare(address, Qt::CaseInsensitive) == 0) {
            return; // already a recipient
        }
    }

    QFrame* widget = createChip(displayName, address, resolved);
    // Insert before the typing area, which stays last so typing continues after
    // the final chip.
    layout()->removeWidget(m_input);
    layout()->addWidget(widget);
    layout()->addWidget(m_input);
    m_chips.append({ displayName, address, resolved, widget });
    emit recipientsChanged();
}

void RecipientChipEdit::removeChip(QFrame* widget)
{
    for (int i = 0; i < m_chips.size(); ++i) {
        if (m_chips.at(i).widget == widget) {
            m_chips.remove(i);
            layout()->removeWidget(widget);
            widget->deleteLater();
            m_input->setFocus();
            emit recipientsChanged();
            return;
        }
    }
}

void RecipientChipEdit::removeLastChip()
{
    if (!m_chips.isEmpty()) {
        removeChip(m_chips.last().widget);
    }
}

QFrame* RecipientChipEdit::createChip(const QString& displayName,
                                      const QString& address,
                                      bool resolved)
{
    auto* chip = new QFrame(this);
    chip->setFrameShape(QFrame::NoFrame);

    auto* row = new QHBoxLayout(chip);
    row->setContentsMargins(6, 1, 1, 1);
    row->setSpacing(2);

    // Both name and address, always: two people in one organization can share a
    // display name, so a name alone cannot identify a recipient (KD7).
    QString text = displayName.isEmpty()
                     ? address
                     : QStringLiteral("%1 <%2>").arg(displayName, address);
    if (!resolved) {
        text = QStringLiteral("⚠ ") + text;
        chip->setToolTip(
          tr("Not found in your organization's directory. It will still be "
             "shared with, in case it is an external address."));
    }
    row->addWidget(new QLabel(text, chip));

    auto* remove = new QToolButton(chip);
    remove->setText(QStringLiteral("×"));
    remove->setAutoRaise(true);
    remove->setFocusPolicy(Qt::NoFocus);
    remove->setToolTip(tr("Remove %1").arg(address));
    connect(remove, &QToolButton::clicked, this, [this, chip]() {
        removeChip(chip);
    });
    row->addWidget(remove);

    const QColor border =
      resolved ? palette().color(QPalette::Mid) : unresolvedBorder();
    chip->setStyleSheet(
      QStringLiteral("QFrame { border: 1px solid %1; border-radius: 7px; "
                     "background-color: %2; }")
        .arg(border.name(), palette().color(QPalette::AlternateBase).name()));
    return chip;
}

void RecipientChipEdit::setInputText(const QString& text)
{
    // setText() does not emit textEdited, so this cannot re-enter onTextEdited.
    m_input->setText(text);
}

void RecipientChipEdit::hidePopup()
{
    if (m_completer->popup() != nullptr) {
        m_completer->popup()->hide();
    }
}

QString RecipientChipEdit::suggestionLabel(
  const RecipientSuggestion& suggestion)
{
    if (suggestion.displayName.isEmpty()) {
        return suggestion.address;
    }
    return QStringLiteral("%1 <%2>").arg(suggestion.displayName,
                                         suggestion.address);
}
