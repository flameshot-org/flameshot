// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "qrwidget.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScreen>
#include <QShortcut>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

QrWidget::QrWidget(const QString& scannedContent,
                   const QString& symbolType,
                   const QPixmap& capturePixmap,
                   QWidget* parent)
  : QWidget(parent)
  , m_content(scannedContent)
  , m_symbolType(symbolType)
  , m_capturePixmap(capturePixmap)
  , m_toastTimer(new QTimer(this))
{
    setWindowIcon(QIcon(QStringLiteral(":/qrplugin/icons/white_qr.svg")));
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                   Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Flameshot QR & Barcode Scanner"));
    setFocusPolicy(Qt::StrongFocus);

    setupUi();
    applyTheme();

    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this, SLOT(close()));

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeom = screen ? screen->geometry() : QRect(0, 0, 1920, 1080);
    int targetW = 620;
    int targetH = 380;
    resize(targetW, targetH);
    move(screenGeom.x() + (screenGeom.width() - targetW) / 2,
         screenGeom.y() + (screenGeom.height() - targetH) / 2);

    if (!m_content.isEmpty()) {
        showToast(tr("Scanned content copied to clipboard!"));
    }
}

void QrWidget::setupUi()
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);

    auto* mainContainer = new QWidget(this);
    mainContainer->setObjectName(QStringLiteral("mainContainer"));
    auto* containerLayout = new QVBoxLayout(mainContainer);
    containerLayout->setContentsMargins(16, 14, 16, 14);
    containerLayout->setSpacing(10);

    // 1. Header
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    auto* iconLabel = new QLabel(this);
    iconLabel->setPixmap(
      QIcon(QStringLiteral(":/qrplugin/icons/white_qr.svg")).pixmap(20, 20));
    headerLayout->addWidget(iconLabel);

    auto* titleLabel = new QLabel(tr("QR & Barcode Scanner"), this);
    titleLabel->setObjectName(QStringLiteral("titleLabel"));
    headerLayout->addWidget(titleLabel);

    m_typeBadge = new QLabel(
      m_symbolType.isEmpty() ? tr("No Code Detected") : m_symbolType, this);
    m_typeBadge->setObjectName(QStringLiteral("typeBadge"));
    headerLayout->addWidget(m_typeBadge);

    if (!m_content.isEmpty()) {
        auto* charBadge =
          new QLabel(tr("%1 chars").arg(m_content.length()), this);
        charBadge->setObjectName(QStringLiteral("typeBadge"));
        headerLayout->addWidget(charBadge);
    }

    headerLayout->addStretch();

    auto* closeBtn = new QPushButton(QStringLiteral("✕"), this);
    closeBtn->setObjectName(QStringLiteral("closeBtn"));
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setToolTip(tr("Close (Esc)"));
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    headerLayout->addWidget(closeBtn);

    containerLayout->addLayout(headerLayout);

    // 2. Toast
    m_toastLabel = new QLabel(this);
    m_toastLabel->setObjectName(QStringLiteral("toastLabel"));
    m_toastLabel->setAlignment(Qt::AlignCenter);
    m_toastLabel->setVisible(false);
    containerLayout->addWidget(m_toastLabel);

    connect(m_toastTimer, &QTimer::timeout, this, [this]() {
        m_toastLabel->setVisible(false);
        m_toastTimer->stop();
    });

    // 3. Central Content
    auto* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(14);

    // Left: Captured Image Preview
    m_previewLabel = new QLabel(this);
    m_previewLabel->setObjectName(QStringLiteral("previewLabel"));
    m_previewLabel->setFixedSize(200, 200);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    if (!m_capturePixmap.isNull()) {
        m_previewLabel->setPixmap(m_capturePixmap.scaled(
          190, 190, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    bodyLayout->addWidget(m_previewLabel);

    // Right: Decoded Content
    auto* rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(6);

    auto* resultTitle = new QLabel(tr("Decoded Payload / Content:"), this);
    resultTitle->setObjectName(QStringLiteral("subTitle"));
    rightLayout->addWidget(resultTitle);

    m_textEdit = new QPlainTextEdit(this);
    m_textEdit->setObjectName(QStringLiteral("textEdit"));
    m_textEdit->setPlainText(
      m_content.isEmpty()
        ? tr("No QR code or barcode was recognized in the selection.")
        : m_content);
    rightLayout->addWidget(m_textEdit, 1);

    bodyLayout->addLayout(rightLayout, 1);
    containerLayout->addLayout(bodyLayout, 1);

    // 4. Bottom Action Bar
    auto* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(8);

    auto* copyBtn = new QPushButton(tr("📋 Copy Text"), this);
    copyBtn->setObjectName(QStringLiteral("actionBtn"));
    copyBtn->setCursor(Qt::PointingHandCursor);
    connect(copyBtn, &QPushButton::clicked, this, &QrWidget::copyText);
    toolbarLayout->addWidget(copyBtn);

    auto* copyImgBtn = new QPushButton(tr("🖼️ Copy Selection"), this);
    copyImgBtn->setObjectName(QStringLiteral("actionBtn"));
    copyImgBtn->setCursor(Qt::PointingHandCursor);
    connect(
      copyImgBtn, &QPushButton::clicked, this, &QrWidget::copyCaptureImage);
    toolbarLayout->addWidget(copyImgBtn);

    m_openBtn = new QPushButton(tr("🔗 Open in Browser"), this);
    m_openBtn->setObjectName(QStringLiteral("openBtn"));
    m_openBtn->setCursor(Qt::PointingHandCursor);
    bool isUrl = m_content.trimmed().startsWith(QStringLiteral("http://"),
                                                Qt::CaseInsensitive) ||
                 m_content.trimmed().startsWith(QStringLiteral("https://"),
                                                Qt::CaseInsensitive);
    m_openBtn->setVisible(isUrl);
    connect(m_openBtn, &QPushButton::clicked, this, &QrWidget::openUrl);
    toolbarLayout->addWidget(m_openBtn);

    toolbarLayout->addStretch();

    auto* doneBtn = new QPushButton(tr("Done"), this);
    doneBtn->setObjectName(QStringLiteral("doneBtn"));
    doneBtn->setFixedSize(70, 32);
    doneBtn->setCursor(Qt::PointingHandCursor);
    connect(doneBtn, &QPushButton::clicked, this, &QWidget::close);
    toolbarLayout->addWidget(doneBtn);

    containerLayout->addLayout(toolbarLayout);
    rootLayout->addWidget(mainContainer);

    // Drop shadow
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setColor(QColor(0, 0, 0, 160));
    shadow->setBlurRadius(22);
    shadow->setOffset(0, 6);
    mainContainer->setGraphicsEffect(shadow);
}

void QrWidget::applyTheme()
{
    setStyleSheet(
      QStringLiteral("#mainContainer {"
                     "    background-color: #1e222b;"
                     "    border: 1px solid #333842;"
                     "    border-radius: 10px;"
                     "}"
                     "#titleLabel {"
                     "    color: #ffffff;"
                     "    font-size: 14px;"
                     "    font-weight: bold;"
                     "}"
                     "#typeBadge {"
                     "    color: #abb2bf;"
                     "    background-color: #282c34;"
                     "    border: 1px solid #3e4451;"
                     "    border-radius: 10px;"
                     "    padding: 2px 8px;"
                     "    font-size: 11px;"
                     "}"
                     "#subTitle {"
                     "    color: #abb2bf;"
                     "    font-size: 11px;"
                     "    font-weight: bold;"
                     "}"
                     "#closeBtn {"
                     "    background: transparent;"
                     "    color: #abb2bf;"
                     "    border: none;"
                     "    border-radius: 14px;"
                     "    font-size: 14px;"
                     "}"
                     "#closeBtn:hover {"
                     "    background-color: #e06c75;"
                     "    color: #ffffff;"
                     "}"
                     "#toastLabel {"
                     "    background-color: #98c379;"
                     "    color: #1e222b;"
                     "    font-weight: bold;"
                     "    font-size: 12px;"
                     "    padding: 4px 12px;"
                     "    border-radius: 6px;"
                     "}"
                     "#previewLabel {"
                     "    background-color: #16181d;"
                     "    border: 1px solid #333842;"
                     "    border-radius: 8px;"
                     "}"
                     "#textEdit {"
                     "    background-color: #21252b;"
                     "    color: #d7dae0;"
                     "    border: 1px solid #333842;"
                     "    border-radius: 6px;"
                     "    padding: 10px;"
                     "    font-size: 13px;"
                     "    line-height: 1.5;"
                     "}"
                     "#actionBtn, #doneBtn {"
                     "    background-color: #282c34;"
                     "    color: #d7dae0;"
                     "    border: 1px solid #3e4451;"
                     "    border-radius: 6px;"
                     "    padding: 6px 14px;"
                     "    font-size: 12px;"
                     "    font-weight: 500;"
                     "}"
                     "#actionBtn:hover, #doneBtn:hover {"
                     "    background-color: #3e4451;"
                     "    color: #ffffff;"
                     "    border: 1px solid #7400b8;"
                     "}"
                     "#openBtn {"
                     "    background-color: #7400b8;"
                     "    color: #ffffff;"
                     "    border: 1px solid rgba(255, 255, 255, 0.3);"
                     "    border-radius: 6px;"
                     "    padding: 6px 14px;"
                     "    font-size: 12px;"
                     "    font-weight: bold;"
                     "}"
                     "#openBtn:hover {"
                     "    background-color: #9b2226;"
                     "}"));
}

void QrWidget::showToast(const QString& msg)
{
    m_toastLabel->setText(msg);
    m_toastLabel->setVisible(true);
    m_toastTimer->start(2800);
}

void QrWidget::openUrl()
{
    QString txt = m_content.trimmed();
    if (!txt.isEmpty()) {
        QDesktopServices::openUrl(QUrl(txt));
    }
}

void QrWidget::copyText()
{
    if (!m_content.isEmpty()) {
        QApplication::clipboard()->setText(m_content);
        showToast(tr("Text copied to clipboard!"));
    }
}

void QrWidget::copyCaptureImage()
{
    if (!m_capturePixmap.isNull()) {
        QApplication::clipboard()->setPixmap(m_capturePixmap);
        showToast(tr("Screenshot image copied to clipboard!"));
    }
}

void QrWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPos =
          event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
}

void QrWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPos);
    }
}

void QrWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
}

void QrWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape ||
        (event->modifiers() & Qt::ControlModifier &&
         event->key() == Qt::Key_Q)) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}
