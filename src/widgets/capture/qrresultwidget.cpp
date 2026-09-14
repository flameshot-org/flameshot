// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Flameshot Contributors

#include "qrresultwidget.h"

#ifdef ENABLE_QR_DECODER

#include "utils/confighandler.h"
#include "utils/colorutils.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QScreen>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------
// Auto-hide timeout (milliseconds)
// ---------------------------------------------------------------------------
static constexpr int AUTO_HIDE_MS = 10000; // 10 seconds

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

QrResultWidget::QrResultWidget(QWidget* parent)
    : QWidget(parent)
    , m_contentType(QrContentType::UNKNOWN)
{
    // --- window flags: frameless, always on top, parented overlay ---
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_StyledBackground, true);

    // --- build UI ---
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 8, 10, 10);
    rootLayout->setSpacing(6);

    // Title row: type label + close button
    auto* titleRow = new QHBoxLayout();
    titleRow->setSpacing(4);

    m_typeLabel = new QLabel(this);
    m_typeLabel->setObjectName(QStringLiteral("typeLabel"));
    m_typeLabel->setWordWrap(false);
    titleRow->addWidget(m_typeLabel, 1);

    m_closeButton = new QPushButton(QStringLiteral("✕"), this);
    m_closeButton->setObjectName(QStringLiteral("closeButton"));
    m_closeButton->setFixedSize(22, 22);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setToolTip(tr("Dismiss"));
    titleRow->addWidget(m_closeButton, 0);

    rootLayout->addLayout(titleRow);

    // Content area
    m_contentLabel = new QLabel(this);
    m_contentLabel->setObjectName(QStringLiteral("contentLabel"));
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_contentLabel->setMaximumWidth(300);
    rootLayout->addWidget(m_contentLabel);

    // Action buttons row
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);
    btnRow->setContentsMargins(0, 4, 0, 0);

    m_copyButton = new QPushButton(tr("📋 Copy"), this);
    m_copyButton->setObjectName(QStringLiteral("copyButton"));
    m_copyButton->setCursor(Qt::PointingHandCursor);
    m_copyButton->setMinimumHeight(28);
    m_copyButton->setMinimumWidth(80);
    btnRow->addWidget(m_copyButton);

    m_openButton = new QPushButton(tr("🌐 Open"), this);
    m_openButton->setObjectName(QStringLiteral("openButton"));
    m_openButton->setCursor(Qt::PointingHandCursor);
    m_openButton->setMinimumHeight(28);
    m_openButton->setMinimumWidth(80);
    m_openButton->setVisible(false); // shown only for URL/EMAIL/PHONE
    btnRow->addWidget(m_openButton);

    btnRow->addStretch(1);

    rootLayout->addLayout(btnRow);

    // --- auto-hide timer ---
    m_autoHideTimer = new QTimer(this);
    m_autoHideTimer->setSingleShot(true);
    m_autoHideTimer->setInterval(AUTO_HIDE_MS);
    connect(m_autoHideTimer, &QTimer::timeout, this, &QrResultWidget::hideResult);

    // --- connect buttons ---
    connect(m_closeButton, &QPushButton::clicked, this, &QrResultWidget::onCloseClicked);
    connect(m_copyButton,  &QPushButton::clicked, this, &QrResultWidget::onCopyClicked);
    connect(m_openButton,  &QPushButton::clicked, this, &QrResultWidget::onOpenClicked);

    // --- initial styling ---
    applyStyleSheet();
    hide();
}

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void QrResultWidget::showResult(const QString& content, const QRect& selectionGeometry)
{
    if (content.isEmpty()) {
        return;
    }

    m_rawContent  = content;
    m_contentType = detectContentType(content);

    // Update labels
    m_typeLabel->setText(QStringLiteral("🔍 QR: ") + contentTypeLabel(m_contentType));
    m_contentLabel->setText(formatContent(content, m_contentType));
    m_contentLabel->setToolTip(content);

    // Show/hide "Open" button
    updateButtons();

    // Re-apply stylesheet (colour may have changed)
    applyStyleSheet();

    // Size to content then position
    adjustSize();
    setMaximumWidth(320);

    positionRelativeToSelection(selectionGeometry);
    show();
    raise();

    // Restart auto-hide
    m_autoHideTimer->start();
}

void QrResultWidget::hideResult()
{
    m_autoHideTimer->stop();
    hide();
    emit dismissed();
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void QrResultWidget::onCopyClicked()
{
    QApplication::clipboard()->setText(m_rawContent);
    emit copyRequested(m_rawContent);

    // Brief visual feedback: change button text then restore
    m_copyButton->setText(tr("✓ Copied!"));
    QTimer::singleShot(1500, this, [this]() {
        m_copyButton->setText(tr("📋 Copy"));
    });
}

void QrResultWidget::onOpenClicked()
{
    QUrl url;
    switch (m_contentType) {
    case QrContentType::URL:
        url = QUrl(m_rawContent);
        break;
    case QrContentType::EMAIL:
        url = QUrl(m_rawContent.startsWith(QStringLiteral("mailto:"), Qt::CaseInsensitive)
                       ? m_rawContent
                       : QStringLiteral("mailto:") + m_rawContent);
        break;
    case QrContentType::PHONE:
        url = QUrl(m_rawContent.startsWith(QStringLiteral("tel:"), Qt::CaseInsensitive)
                       ? m_rawContent
                       : QStringLiteral("tel:") + m_rawContent);
        break;
    default:
        return;
    }

    if (url.isValid()) {
        QDesktopServices::openUrl(url);
        emit openUrlRequested(url);
    }
}

void QrResultWidget::onCloseClicked()
{
    hideResult();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

QrResultWidget::QrContentType QrResultWidget::detectContentType(const QString& content) const
{
    // URL
    QUrl url(content);
    if (url.isValid()) {
        const QString scheme = url.scheme().toLower();
        if (scheme == QLatin1String("http")  ||
            scheme == QLatin1String("https") ||
            scheme == QLatin1String("ftp")) {
            return QrContentType::URL;
        }
    }

    // WiFi: WIFI:T:WPA;S:...;P:...;;
    if (content.startsWith(QStringLiteral("WIFI:"), Qt::CaseInsensitive)) {
        return QrContentType::WIFI;
    }

    // vCard
    if (content.startsWith(QStringLiteral("BEGIN:VCARD"), Qt::CaseInsensitive)) {
        return QrContentType::VCARD;
    }

    // Email: MAILTO: prefix or bare address pattern
    if (content.startsWith(QStringLiteral("mailto:"), Qt::CaseInsensitive)) {
        return QrContentType::EMAIL;
    }
    static const QRegularExpression emailRe(
        QStringLiteral("^[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}$"));
    if (emailRe.match(content).hasMatch()) {
        return QrContentType::EMAIL;
    }

    // Phone: TEL: or tel: scheme
    if (content.startsWith(QStringLiteral("TEL:"),  Qt::CaseInsensitive) ||
        content.startsWith(QStringLiteral("tel:"),  Qt::CaseInsensitive)) {
        return QrContentType::PHONE;
    }

    // SMS
    if (content.startsWith(QStringLiteral("sms:"),   Qt::CaseInsensitive) ||
        content.startsWith(QStringLiteral("smsto:"), Qt::CaseInsensitive)) {
        return QrContentType::SMS;
    }

    // Geo
    if (content.startsWith(QStringLiteral("geo:"), Qt::CaseInsensitive)) {
        return QrContentType::GEO;
    }

    return QrContentType::TEXT;
}

QString QrResultWidget::formatContent(const QString& content, QrContentType type) const
{
    switch (type) {
    case QrContentType::WIFI: {
        // Parse: WIFI:T:WPA;S:MyNetwork;P:password;;
        static const QRegularExpression wifiRe(
            QStringLiteral("WIFI:T:([^;]*);S:([^;]*);P:([^;]*)"),
            QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch m = wifiRe.match(content);
        if (m.hasMatch()) {
            return tr("SSID: %1\nType: %2\nPassword: %3")
                .arg(m.captured(2), m.captured(1), m.captured(3));
        }
        break;
    }
    case QrContentType::VCARD: {
        // Extract FN (full name) and TEL / EMAIL if present
        QString name, phone, email;
        for (const QString& line : content.split(QLatin1Char('\n'))) {
            const QString t = line.trimmed();
            if (t.startsWith(QStringLiteral("FN:"), Qt::CaseInsensitive))
                name = t.mid(3);
            else if (t.startsWith(QStringLiteral("TEL"), Qt::CaseInsensitive))
                phone = t.mid(t.indexOf(QLatin1Char(':')) + 1);
            else if (t.startsWith(QStringLiteral("EMAIL"), Qt::CaseInsensitive))
                email = t.mid(t.indexOf(QLatin1Char(':')) + 1);
        }
        QString result;
        if (!name.isEmpty())  result += tr("Name: %1\n").arg(name);
        if (!phone.isEmpty()) result += tr("Tel: %1\n").arg(phone);
        if (!email.isEmpty()) result += tr("Email: %1").arg(email);
        return result.trimmed().isEmpty() ? content : result.trimmed();
    }
    default:
        break;
    }

    // For everything else, truncate very long content
    if (content.length() > 200) {
        return content.left(197) + QStringLiteral("...");
    }
    return content;
}

QString QrResultWidget::contentTypeLabel(QrContentType type) const
{
    switch (type) {
    case QrContentType::URL:     return tr("URL");
    case QrContentType::WIFI:    return tr("WiFi");
    case QrContentType::VCARD:   return tr("vCard");
    case QrContentType::EMAIL:   return tr("Email");
    case QrContentType::PHONE:   return tr("Phone");
    case QrContentType::SMS:     return tr("SMS");
    case QrContentType::GEO:     return tr("Location");
    case QrContentType::TEXT:    return tr("Text");
    default:                     return tr("Unknown");
    }
}

void QrResultWidget::updateButtons()
{
    // "Open" button is only meaningful for URL, EMAIL, PHONE
    bool showOpen = (m_contentType == QrContentType::URL  ||
                     m_contentType == QrContentType::EMAIL ||
                     m_contentType == QrContentType::PHONE);
    m_openButton->setVisible(showOpen);

    if (m_contentType == QrContentType::URL) {
        m_openButton->setText(tr("🌐 Open"));
    } else if (m_contentType == QrContentType::EMAIL) {
        m_openButton->setText(tr("✉ Mail"));
    } else if (m_contentType == QrContentType::PHONE) {
        m_openButton->setText(tr("📞 Call"));
    }
}

void QrResultWidget::positionRelativeToSelection(const QRect& selection)
{
    QRect availableBounds = parentWidget() ? parentWidget()->rect() : QRect(0, 0, 1920, 1080);

    // If multi-monitor setup, constrain available bounds to the specific screen where selection sits
    if (parentWidget()) {
        const QPoint globalCenter = parentWidget()->mapToGlobal(selection.center());
        QScreen* targetScreen = QGuiApplication::screenAt(globalCenter);
        if (!targetScreen) {
            targetScreen = parentWidget()->screen();
        }
        if (targetScreen) {
            const QRect screenGeom = targetScreen->availableGeometry();
            const QPoint localTopLeft = parentWidget()->mapFromGlobal(screenGeom.topLeft());
            const QRect localScreen(localTopLeft, screenGeom.size());
            availableBounds = availableBounds.intersected(localScreen);
        }
    }

    const int margin = 10;
    const int w      = width()  > 0 ? width()  : 320;
    const int h      = height() > 0 ? height() : 200;

    // Priority: right → left → below → above
    QRect rightPos(selection.right() + margin, selection.top(), w, h);
    if (availableBounds.contains(rightPos)) {
        move(rightPos.topLeft());
        return;
    }

    QRect leftPos(selection.left() - w - margin, selection.top(), w, h);
    if (availableBounds.contains(leftPos)) {
        move(leftPos.topLeft());
        return;
    }

    QRect belowPos(selection.left(), selection.bottom() + margin, w, h);
    if (availableBounds.contains(belowPos)) {
        move(belowPos.topLeft());
        return;
    }

    QRect abovePos(selection.left(), selection.top() - h - margin, w, h);
    if (availableBounds.contains(abovePos)) {
        move(abovePos.topLeft());
        return;
    }

    // Fallback: clamp inside availableBounds near selection
    int x = qBound(availableBounds.left(), selection.left(), qMax(availableBounds.left(), availableBounds.right() - w));
    int y = qBound(availableBounds.top(), selection.top(), qMax(availableBounds.top(), availableBounds.bottom() - h));
    move(x, y);
}

void QrResultWidget::applyStyleSheet()
{
    const QColor uiColor = ConfigHandler().uiColor();
    const QColor bgColor = uiColor.lighter(170);
    const QColor textColor = ColorUtils::colorIsDark(bgColor) ? Qt::white : Qt::black;
    const QColor btnTextColor = ColorUtils::colorIsDark(uiColor) ? Qt::white : Qt::black;
    const QColor borderColor = uiColor.darker(120);
    const QColor hoverColor = uiColor.lighter(118);
    const QColor pressedColor = uiColor.darker(115);

    const QString bgHex      = bgColor.name();
    const QString uiHex      = uiColor.name();
    const QString txtHex     = textColor.name();
    const QString borderHex  = borderColor.name();
    const QString btnTxtHex  = btnTextColor.name();
    const QString hoverHex   = hoverColor.name();
    const QString pressedHex = pressedColor.name();

    setStyleSheet(QStringLiteral(R"(
        QrResultWidget {
            background-color: %1;
            border: 2px solid %4;
            border-radius: 8px;
        }
        QLabel#typeLabel {
            font-weight: bold;
            font-size: 12px;
            color: %2;
        }
        QLabel#contentLabel {
            background-color: rgba(0, 0, 0, 0.08);
            border: 1px solid %4;
            border-radius: 4px;
            padding: 6px 8px;
            font-family: monospace;
            font-size: 11px;
            color: %3;
        }
        QPushButton#copyButton, QPushButton#openButton {
            background-color: %2;
            color: %5;
            border: 1px solid %4;
            border-radius: 4px;
            padding: 4px 12px;
            font-size: 11px;
            font-weight: bold;
        }
        QPushButton#copyButton:hover, QPushButton#openButton:hover {
            background-color: %6;
            color: %5;
        }
        QPushButton#copyButton:pressed, QPushButton#openButton:pressed {
            background-color: %7;
            color: %5;
        }
        QPushButton#closeButton {
            background-color: transparent;
            border: none;
            border-radius: 11px;
            font-size: 13px;
            color: %3;
        }
        QPushButton#closeButton:hover {
            background-color: rgba(0, 0, 0, 0.15);
        }
    )")
    .arg(bgHex, uiHex, txtHex, borderHex, btnTxtHex, hoverHex, pressedHex));
}

#endif // ENABLE_QR_DECODER
