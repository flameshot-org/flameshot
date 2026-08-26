// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "ocrwidget.h"
#include "utils/colorutils.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLoggingCategory>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
#include <QSplitter>
#include <QStyle>
#include <QTextStream>
#include <QTimer>
#include <QToolTip>
#include <QUrl>
#include <QVBoxLayout>
#include <QWheelEvent>



// ============================================================================
// OcrCanvas Implementation (Interactive Preview + Zoom + Highlights)
// ============================================================================

OcrCanvas::OcrCanvas(const QPixmap& pixmap,
                     const QVector<QRect>& wordBoxes,
                     const QStringList& words,
                     const QVector<QRect>& lineBoxes,
                     const QStringList& lines,
                     QWidget* parent)
  : QWidget(parent)
  , m_pixmap(pixmap)
  , m_wordBoxes(wordBoxes)
  , m_words(words)
  , m_lineBoxes(lineBoxes)
  , m_lines(lines)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    updateCanvasSize();
}

void OcrCanvas::updateCanvasSize()
{
    if (m_pixmap.isNull()) {
        setFixedSize(360, 260);
        return;
    }
    int w = qMax(120, qRound(m_pixmap.width() * m_zoomFactor));
    int h = qMax(90, qRound(m_pixmap.height() * m_zoomFactor));
    setFixedSize(w, h);
}

QSize OcrCanvas::sizeHint() const
{
    if (m_pixmap.isNull()) {
        return QSize(400, 300);
    }
    return QSize(qMax(120, qRound(m_pixmap.width() * m_zoomFactor)),
                 qMax(90, qRound(m_pixmap.height() * m_zoomFactor)));
}

void OcrCanvas::setFilterQuery(const QString& query)
{
    m_filterQuery = query.trimmed();
    update();
}

void OcrCanvas::setHighlightBoxes(bool enabled)
{
    m_showBoxes = enabled;
    update();
}

void OcrCanvas::selectAllWords()
{
    m_selectedIndices.clear();
    for (int i = 0; i < m_words.size(); ++i) {
        m_selectedIndices.insert(i);
    }
    emit selectionChanged();
    update();
}

void OcrCanvas::clearSelection()
{
    m_selectedIndices.clear();
    emit selectionChanged();
    update();
}

QString OcrCanvas::selectedText() const
{
    if (m_selectedIndices.isEmpty()) {
        return QString();
    }
    QStringList sel;
    for (int i = 0; i < m_words.size(); ++i) {
        if (m_selectedIndices.contains(i)) {
            sel.append(m_words.at(i));
        }
    }
    return sel.join(QStringLiteral(" "));
}

void OcrCanvas::zoomIn()
{
    setZoomFactor(m_zoomFactor * 1.25);
}

void OcrCanvas::zoomOut()
{
    setZoomFactor(m_zoomFactor / 1.25);
}

void OcrCanvas::zoomFit()
{
    m_panOffset = QPoint(0, 0);
    setZoomFactor(1.0);
}

void OcrCanvas::setZoomFactor(qreal factor)
{
    qreal newZoom = qBound(0.25, factor, 6.0);
    if (!qFuzzyCompare(m_zoomFactor, newZoom)) {
        m_zoomFactor = newZoom;
        updateCanvasSize();
        emit zoomChanged(m_zoomFactor);
        update();
    }
}

QRect OcrCanvas::mapToCanvas(const QRect& r) const
{
    return QRect(qRound(r.x() * m_zoomFactor),
                 qRound(r.y() * m_zoomFactor),
                 qMax(2, qRound(r.width() * m_zoomFactor)),
                 qMax(2, qRound(r.height() * m_zoomFactor)));
}

QRect OcrCanvas::mapFromCanvas(const QRect& r) const
{
    if (m_zoomFactor <= 0.0) {
        return QRect();
    }
    return QRect(qRound(r.x() / m_zoomFactor),
                 qRound(r.y() / m_zoomFactor),
                 qRound(r.width() / m_zoomFactor),
                 qRound(r.height() / m_zoomFactor));
}

int OcrCanvas::findWordAt(const QPoint& pos) const
{
    for (int i = 0; i < m_wordBoxes.size(); ++i) {
        QRect canvasRect = mapToCanvas(m_wordBoxes.at(i));
        if (canvasRect.adjusted(-3, -3, 3, 3).contains(pos)) {
            return i;
        }
    }
    return -1;
}

void OcrCanvas::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // Canvas background
    p.fillRect(rect(), QColor(22, 24, 29));

    if (m_pixmap.isNull()) {
        return;
    }

    // Draw capture image scaled
    p.drawPixmap(rect(), m_pixmap);

    // Subtle dark overlay to let bounding boxes pop
    p.fillRect(rect(), QColor(0, 0, 0, 20));

    const QColor baseColor = QColor(116, 0, 180);

    if (m_showBoxes) {
        // Draw word boxes
        for (int i = 0; i < m_wordBoxes.size(); ++i) {
            QRect cRect = mapToCanvas(m_wordBoxes.at(i));
            if (!cRect.isValid() || !rect().intersects(cRect))
                continue;

            bool isSelected = m_selectedIndices.contains(i);
            bool isHovered = (i == m_hoveredIndex);
            bool isSearchMatch =
              (!m_filterQuery.isEmpty() &&
               m_words.value(i).contains(m_filterQuery, Qt::CaseInsensitive));

            QPainterPath path;
            path.addRoundedRect(cRect, 3, 3);

            if (isSearchMatch) {
                // Gold / Amber glow for search matches with high contrast
                p.fillPath(path, QColor(255, 193, 7, 180));
                p.strokePath(path, QPen(QColor(255, 235, 59), 2.5));
                p.setPen(QColor(0, 0, 0));
                p.setFont(QFont(QStringLiteral("sans-serif"), qMax(8, qRound(10 * m_zoomFactor)), QFont::Bold));
                p.drawText(cRect, Qt::AlignCenter, m_words.value(i));
            } else if (isSelected) {
                // Vivid accent for selected words
                p.fillPath(
                  path,
                  QColor(
                    baseColor.red(), baseColor.green(), baseColor.blue(), 130));
                p.strokePath(path, QPen(baseColor.lighter(130), 1.8));
            } else if (isHovered) {
                // Bright hover box
                p.fillPath(
                  path,
                  QColor(
                    baseColor.red(), baseColor.green(), baseColor.blue(), 80));
                p.strokePath(path, QPen(baseColor.lighter(150), 1.5));
            } else {
                // Subtle ambient box (dimmed when active search is filtering)
                int alpha = (!m_filterQuery.isEmpty()) ? 10 : 25;
                p.fillPath(
                  path,
                  QColor(
                    baseColor.red(), baseColor.green(), baseColor.blue(), alpha));
                p.strokePath(path,
                             QPen(QColor(baseColor.red(),
                                         baseColor.green(),
                                         baseColor.blue(),
                                         (!m_filterQuery.isEmpty()) ? 40 : 90),
                                  1));
            }
        }
    }

    // Drag selection marquee box
    if (m_isSelecting) {
        QRect selRect =
          QRect(m_selectionStart, m_selectionCurrent).normalized();
        p.fillRect(
          selRect,
          QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 50));
        p.setPen(QPen(baseColor, 1.5, Qt::DashLine));
        p.drawRect(selRect);
    }
}

void OcrCanvas::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        const int delta = event->angleDelta().y();
        if (delta > 0) {
            zoomIn();
        } else if (delta < 0) {
            zoomOut();
        }
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

void OcrCanvas::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_selectionStart = event->pos();
        m_selectionCurrent = event->pos();
        m_isSelecting = true;

        int idx = findWordAt(event->pos());
        if (idx >= 0) {
            if (event->modifiers() & Qt::ControlModifier) {
                if (m_selectedIndices.contains(idx)) {
                    m_selectedIndices.remove(idx);
                } else {
                    m_selectedIndices.insert(idx);
                }
            } else {
                m_selectedIndices.clear();
                m_selectedIndices.insert(idx);
            }
            emit selectionChanged();
            update();
        } else if (!(event->modifiers() & Qt::ControlModifier)) {
            m_selectedIndices.clear();
            emit selectionChanged();
            update();
        }
    }
}

void OcrCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isSelecting) {
        m_selectionCurrent = event->pos();
        QRect selRect =
          QRect(m_selectionStart, m_selectionCurrent).normalized();

        for (int i = 0; i < m_wordBoxes.size(); ++i) {
            QRect cRect = mapToCanvas(m_wordBoxes.at(i));
            if (selRect.intersects(cRect)) {
                m_selectedIndices.insert(i);
            }
        }
        emit selectionChanged();
        update();
        return;
    }

    int idx = findWordAt(event->pos());
    if (idx != m_hoveredIndex) {
        m_hoveredIndex = idx;
        if (idx >= 0 && idx < m_words.size()) {
            QToolTip::showText(event->globalPosition().toPoint(),
                               m_words.at(idx),
                               this,
                               mapToCanvas(m_wordBoxes.at(idx)));
        } else {
            QToolTip::hideText();
        }
        update();
    }
}

void OcrCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_isSelecting) {
        m_isSelecting = false;
        update();
    }
}

void OcrCanvas::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        int idx = findWordAt(event->pos());
        if (idx >= 0 && idx < m_words.size()) {
            QString word = m_words.at(idx);
            QApplication::clipboard()->setText(word);
            emit wordDoubleClicked(word);
            emit statusMessageRequested(tr("Word copied: \"%1\"").arg(word));
        }
    }
}

void OcrCanvas::leaveEvent(QEvent*)
{
    if (m_hoveredIndex != -1) {
        m_hoveredIndex = -1;
        QToolTip::hideText();
        update();
    }
}

void OcrCanvas::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    menu.setStyleSheet(QStringLiteral(
      "QMenu { background-color: #23272e; color: #abb2bf; border: 1px solid "
      "#3b4048; border-radius: 6px; padding: 4px; }"
      "QMenu::item { padding: 6px 20px; border-radius: 4px; }"
      "QMenu::item:selected { background-color: #3e4451; color: #ffffff; }"));

    int clickedIdx = findWordAt(event->pos());
    if (clickedIdx >= 0 && !m_selectedIndices.contains(clickedIdx)) {
        m_selectedIndices.clear();
        m_selectedIndices.insert(clickedIdx);
        emit selectionChanged();
        update();
    }

    QString selText = selectedText();
    if (!selText.isEmpty()) {
        QAction* copySelAct = menu.addAction(
          tr("Copy Selected (%1 words)").arg(m_selectedIndices.size()));
        connect(copySelAct, &QAction::triggered, this, [this, selText]() {
            QApplication::clipboard()->setText(selText);
            emit statusMessageRequested(
              tr("Copied %1 words to clipboard").arg(m_selectedIndices.size()));
        });

        QAction* searchSelAct = menu.addAction(tr("Web Search Selected"));
        connect(searchSelAct, &QAction::triggered, this, [selText]() {
            if (selText.startsWith(QStringLiteral("http://"),
                                   Qt::CaseInsensitive) ||
                selText.startsWith(QStringLiteral("https://"),
                                   Qt::CaseInsensitive)) {
                QDesktopServices::openUrl(QUrl(selText.trimmed()));
            } else {
                QUrl url(QStringLiteral("https://www.google.com/search?q=") +
                         QUrl::toPercentEncoding(selText));
                QDesktopServices::openUrl(url);
            }
        });

        QAction* transSelAct = menu.addAction(tr("Translate Selected"));
        connect(transSelAct, &QAction::triggered, this, [selText]() {
            QUrl url(QStringLiteral(
                       "https://translate.google.com/?sl=auto&tl=en&text=") +
                     QUrl::toPercentEncoding(selText));
            QDesktopServices::openUrl(url);
        });

        menu.addSeparator();
    }

    QAction* zoomInAct = menu.addAction(tr("Zoom In (Ctrl+Wheel Up)"));
    connect(zoomInAct, &QAction::triggered, this, &OcrCanvas::zoomIn);

    QAction* zoomOutAct = menu.addAction(tr("Zoom Out (Ctrl+Wheel Down)"));
    connect(zoomOutAct, &QAction::triggered, this, &OcrCanvas::zoomOut);

    QAction* zoomFitAct = menu.addAction(tr("Zoom Fit (100%)"));
    connect(zoomFitAct, &QAction::triggered, this, &OcrCanvas::zoomFit);

    menu.addSeparator();

    QAction* selectAllAct = menu.addAction(tr("Select All Words"));
    connect(
      selectAllAct, &QAction::triggered, this, &OcrCanvas::selectAllWords);

    if (!m_selectedIndices.isEmpty()) {
        QAction* clearSelAct = menu.addAction(tr("Clear Selection"));
        connect(
          clearSelAct, &QAction::triggered, this, &OcrCanvas::clearSelection);
    }

    menu.exec(event->globalPos());
}

// ============================================================================
// OcrWidget Implementation
// ============================================================================

OcrWidget::OcrWidget(const QPixmap& pixmap,
                     const QRect& geometry,
                     const QString& fullText,
                     const QVector<QRect>& wordBoxes,
                     const QStringList& words,
                     const QVector<QRect>& lineBoxes,
                     const QStringList& lines,
                     QWidget* parent)
  : QWidget(parent)
  , m_pixmap(pixmap)
  , m_geometry(geometry)
  , m_fullText(fullText)
  , m_wordBoxes(wordBoxes)
  , m_words(words)
  , m_lineBoxes(lineBoxes)
  , m_lines(lines)
  , m_toastTimer(new QTimer(this))
{
    // Filter out font shaper warning logs
    QLoggingCategory::setFilterRules(
      QStringLiteral("qt.text.font.db.warning=false\nqt.text.font.warning="
                     "false\nqt.text.font=false"));

    setWindowIcon(QIcon(QStringLiteral(":/ocrplugin/icons/white_ocr.svg")));
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                   Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Flameshot Text Recognition (OCR)"));
    setFocusPolicy(Qt::StrongFocus);

    m_accentColor = QColor(116, 0, 180);
    m_contrastColor = QColor(255, 255, 255);

    setupUi();
    applyTheme();

    // Keyboard Shortcuts
    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this, SLOT(close()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_G), this, SLOT(searchWeb()));
    new QShortcut(
      QKeySequence(Qt::CTRL | Qt::Key_T), this, SLOT(translateText()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, SLOT(saveToFile()));

    // Center on screen or near capture geometry
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeom = screen ? screen->geometry() : QRect(0, 0, 1920, 1080);
    int targetW = qBound(700, qRound(screenGeom.width() * 0.58), 1150);
    int targetH = qBound(480, qRound(screenGeom.height() * 0.58), 780);
    resize(targetW, targetH);

    int posX = screenGeom.x() + (screenGeom.width() - targetW) / 2;
    int posY = screenGeom.y() + (screenGeom.height() - targetH) / 2;
    move(posX, posY);

    // Initial toast
    showToast(tr("Text extracted and copied to clipboard!"));
}

void OcrWidget::setupUi()
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);

    auto* mainContainer = new QWidget(this);
    mainContainer->setObjectName(QStringLiteral("mainContainer"));
    auto* containerLayout = new QVBoxLayout(mainContainer);
    containerLayout->setContentsMargins(14, 12, 14, 12);
    containerLayout->setSpacing(8);

    // ------------------------------------------------------------------------
    // 1. Header Bar
    // ------------------------------------------------------------------------
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    auto* iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(QStringLiteral(":/ocrplugin/icons/white_ocr.svg")).pixmap(20, 20));
    headerLayout->addWidget(iconLabel);

    auto* titleLabel = new QLabel(tr("Text Recognition (OCR)"), this);
    titleLabel->setObjectName(QStringLiteral("titleLabel"));
    headerLayout->addWidget(titleLabel);

    m_statsBadge = new QLabel(
      tr("%1 words • %2 lines").arg(m_words.size()).arg(m_lines.size()), this);
    m_statsBadge->setObjectName(QStringLiteral("statsBadge"));
    headerLayout->addWidget(m_statsBadge);

    m_zoomBadge = new QLabel(QStringLiteral("100%"), this);
    m_zoomBadge->setObjectName(QStringLiteral("zoomBadge"));
    headerLayout->addWidget(m_zoomBadge);

    headerLayout->addStretch();

    auto* closeBtn = new QPushButton(QStringLiteral("✕"), this);
    closeBtn->setObjectName(QStringLiteral("closeBtn"));
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setToolTip(tr("Close (Esc)"));
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    headerLayout->addWidget(closeBtn);

    containerLayout->addLayout(headerLayout);

    // ------------------------------------------------------------------------
    // 2. Toast Notification Banner
    // ------------------------------------------------------------------------
    m_toastLabel = new QLabel(this);
    m_toastLabel->setObjectName(QStringLiteral("toastLabel"));
    m_toastLabel->setAlignment(Qt::AlignCenter);
    m_toastLabel->setVisible(false);
    containerLayout->addWidget(m_toastLabel);

    connect(m_toastTimer, &QTimer::timeout, this, [this]() {
        m_toastLabel->setVisible(false);
        m_toastTimer->stop();
    });

    // ------------------------------------------------------------------------
    // 3. Central Splitter (Visual Canvas with Scrollbars + Text Panel)
    // ------------------------------------------------------------------------
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName(QStringLiteral("splitter"));
    splitter->setChildrenCollapsible(false);

    // Left Pane: Visual Canvas Container (with Horizontal & Vertical
    // Scrollbars)
    auto* canvasContainer = new QWidget(this);
    auto* canvasLayout = new QVBoxLayout(canvasContainer);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->setSpacing(4);

    m_canvas =
      new OcrCanvas(m_pixmap, m_wordBoxes, m_words, m_lineBoxes, m_lines, this);
    connect(
      m_canvas, &OcrCanvas::wordDoubleClicked, this, [this](const QString& w) {
          showToast(tr("Copied word: \"%1\"").arg(w));
      });
    connect(m_canvas,
            &OcrCanvas::statusMessageRequested,
            this,
            &OcrWidget::showToast);
    connect(
      m_canvas, &OcrCanvas::zoomChanged, this, &OcrWidget::updateZoomBadge);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setObjectName(QStringLiteral("canvasScrollArea"));
    scrollArea->setWidgetResizable(false);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidget(m_canvas);
    canvasLayout->addWidget(scrollArea, 1);

    // Zoom toolbar directly under preview canvas
    auto* zoomBarLayout = new QHBoxLayout();
    zoomBarLayout->setSpacing(4);

    auto* zoomOutBtn = new QPushButton(QStringLiteral("−"), this);
    zoomOutBtn->setObjectName(QStringLiteral("zoomBtn"));
    zoomOutBtn->setFixedSize(28, 24);
    zoomOutBtn->setCursor(Qt::PointingHandCursor);
    zoomOutBtn->setToolTip(tr("Zoom Out (Ctrl+Wheel Down)"));
    connect(zoomOutBtn, &QPushButton::clicked, m_canvas, &OcrCanvas::zoomOut);

    auto* zoom100Btn = new QPushButton(QStringLiteral("100%"), this);
    zoom100Btn->setObjectName(QStringLiteral("zoomBtn"));
    zoom100Btn->setFixedSize(54, 24);
    zoom100Btn->setCursor(Qt::PointingHandCursor);
    zoom100Btn->setToolTip(tr("Reset Zoom (100%)"));
    m_zoomBtn = zoom100Btn;
    connect(zoom100Btn, &QPushButton::clicked, this, [this]() {
        m_canvas->zoomFit();
    });

    auto* zoomInBtn = new QPushButton(QStringLiteral("+"), this);
    zoomInBtn->setObjectName(QStringLiteral("zoomBtn"));
    zoomInBtn->setFixedSize(28, 24);
    zoomInBtn->setCursor(Qt::PointingHandCursor);
    zoomInBtn->setToolTip(tr("Zoom In (Ctrl+Wheel Up)"));
    connect(zoomInBtn, &QPushButton::clicked, m_canvas, &OcrCanvas::zoomIn);

    auto* zoomFitBtn = new QPushButton(tr("Fit"), this);
    zoomFitBtn->setObjectName(QStringLiteral("zoomBtn"));
    zoomFitBtn->setFixedSize(42, 24);
    zoomFitBtn->setCursor(Qt::PointingHandCursor);
    zoomFitBtn->setToolTip(tr("Fit to View"));
    connect(zoomFitBtn, &QPushButton::clicked, m_canvas, &OcrCanvas::zoomFit);

    zoomBarLayout->addWidget(zoomOutBtn);
    zoomBarLayout->addWidget(zoom100Btn);
    zoomBarLayout->addWidget(zoomInBtn);
    zoomBarLayout->addWidget(zoomFitBtn);
    zoomBarLayout->addStretch();

    canvasLayout->addLayout(zoomBarLayout);
    splitter->addWidget(canvasContainer);

    // Right Pane: Text Editor & Inspector
    auto* rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(4, 0, 0, 0);
    rightLayout->setSpacing(6);

    // Search bar
    m_searchBox = new QLineEdit(this);
    m_searchBox->setObjectName(QStringLiteral("searchBox"));
    m_searchBox->setPlaceholderText(tr("🔍 Find in text..."));
    m_searchBox->setClearButtonEnabled(true);
    connect(m_searchBox,
            &QLineEdit::textChanged,
            this,
            &OcrWidget::onSearchTextChanged);
    rightLayout->addWidget(m_searchBox);

    // Format mode selector tabs
    auto* formatLayout = new QHBoxLayout();
    formatLayout->setSpacing(4);

    m_fmtBtn = new QPushButton(tr("Formatted"), this);
    m_lineBtn = new QPushButton(tr("Lines"), this);
    m_singleBtn = new QPushButton(tr("Single"), this);
    m_tableBtn = new QPushButton(tr("📊 Table (MD)"), this);
    m_codeBtn = new QPushButton(tr("💻 Code"), this);
    m_latexBtn = new QPushButton(tr("∑ LaTeX"), this);

    for (auto* btn : { m_fmtBtn, m_lineBtn, m_singleBtn, m_tableBtn, m_codeBtn, m_latexBtn }) {
        btn->setCheckable(true);
        btn->setObjectName(QStringLiteral("fmtBtn"));
        formatLayout->addWidget(btn);
    }
    m_fmtBtn->setChecked(true);

    auto updateFormatSelection = [this](int mode, QPushButton* activeBtn) {
        for (auto* btn : { m_fmtBtn, m_lineBtn, m_singleBtn, m_tableBtn, m_codeBtn, m_latexBtn }) {
            btn->setChecked(btn == activeBtn);
        }
        setFormatMode(mode);
    };

    connect(m_fmtBtn, &QPushButton::clicked, this, [this, updateFormatSelection]() { updateFormatSelection(0, m_fmtBtn); });
    connect(m_lineBtn, &QPushButton::clicked, this, [this, updateFormatSelection]() { updateFormatSelection(1, m_lineBtn); });
    connect(m_singleBtn, &QPushButton::clicked, this, [this, updateFormatSelection]() { updateFormatSelection(2, m_singleBtn); });
    connect(m_tableBtn, &QPushButton::clicked, this, [this, updateFormatSelection]() { updateFormatSelection(3, m_tableBtn); });
    connect(m_codeBtn, &QPushButton::clicked, this, [this, updateFormatSelection]() { updateFormatSelection(4, m_codeBtn); });
    connect(m_latexBtn, &QPushButton::clicked, this, [this, updateFormatSelection]() { updateFormatSelection(5, m_latexBtn); });

    rightLayout->addLayout(formatLayout);

    // Text Editor
    m_textEdit = new QPlainTextEdit(this);
    m_textEdit->setObjectName(QStringLiteral("textEdit"));
    m_textEdit->setPlainText(m_fullText);
    rightLayout->addWidget(m_textEdit);

    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    containerLayout->addWidget(splitter, 1);

    // ------------------------------------------------------------------------
    // 4. Action Toolbar (Bottom Bar)
    // ------------------------------------------------------------------------
    auto* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(8);

    auto* copyAllBtn = new QPushButton(tr("📋 Copy All"), this);
    copyAllBtn->setObjectName(QStringLiteral("copyAllBtn"));
    copyAllBtn->setCursor(Qt::PointingHandCursor);
    connect(copyAllBtn, &QPushButton::clicked, this, &OcrWidget::copyAllText);
    toolbarLayout->addWidget(copyAllBtn);

    auto* copySelBtn = new QPushButton(tr("🔠 Copy Selected"), this);
    copySelBtn->setObjectName(QStringLiteral("copySelBtn"));
    copySelBtn->setCursor(Qt::PointingHandCursor);
    connect(
      copySelBtn, &QPushButton::clicked, this, &OcrWidget::copySelectedText);
    toolbarLayout->addWidget(copySelBtn);

    auto* searchBtn = new QPushButton(tr("🔍 Web Search"), this);
    searchBtn->setObjectName(QStringLiteral("actionBtn"));
    searchBtn->setCursor(Qt::PointingHandCursor);
    searchBtn->setToolTip(tr("Search on default web browser (Ctrl+G)"));
    connect(searchBtn, &QPushButton::clicked, this, &OcrWidget::searchWeb);
    toolbarLayout->addWidget(searchBtn);

    auto* transBtn = new QPushButton(tr("🌐 Translate"), this);
    transBtn->setObjectName(QStringLiteral("actionBtn"));
    transBtn->setCursor(Qt::PointingHandCursor);
    transBtn->setToolTip(tr("Translate in web browser (Ctrl+T)"));
    connect(transBtn, &QPushButton::clicked, this, &OcrWidget::translateText);
    toolbarLayout->addWidget(transBtn);

    auto* saveBtn = new QPushButton(tr("💾 Save .txt"), this);
    saveBtn->setObjectName(QStringLiteral("actionBtn"));
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setToolTip(tr("Save text to a file (Ctrl+S)"));
    connect(saveBtn, &QPushButton::clicked, this, &OcrWidget::saveToFile);
    toolbarLayout->addWidget(saveBtn);

    auto* pinBtn = new QPushButton(tr("📌 Pin"), this);
    pinBtn->setObjectName(QStringLiteral("actionBtn"));
    pinBtn->setCursor(Qt::PointingHandCursor);
    pinBtn->setToolTip(tr("Pin image to desktop"));
    connect(pinBtn, &QPushButton::clicked, this, &OcrWidget::pinScreenshot);
    toolbarLayout->addWidget(pinBtn);

    toolbarLayout->addStretch();

    auto* doneBtn = new QPushButton(tr("Done"), this);
    doneBtn->setObjectName(QStringLiteral("doneBtn"));
    doneBtn->setCursor(Qt::PointingHandCursor);
    connect(doneBtn, &QPushButton::clicked, this, &QWidget::close);
    toolbarLayout->addWidget(doneBtn);

    containerLayout->addLayout(toolbarLayout);
    rootLayout->addWidget(mainContainer);

    // Drop shadow
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setColor(QColor(0, 0, 0, 160));
    m_shadowEffect->setBlurRadius(20);
    m_shadowEffect->setOffset(0, 6);
    mainContainer->setGraphicsEffect(m_shadowEffect);
}

void OcrWidget::applyTheme()
{
    QString accentHex = m_accentColor.name();
    QString accentHover = m_accentColor.lighter(135).name();
    QString accentPressed = m_accentColor.darker(120).name();

    setStyleSheet(
      QString("#mainContainer {"
              "    background-color: #1e222b;"
              "    border: 1px solid #333842;"
              "    border-radius: 10px;"
              "}"
              "#titleLabel {"
              "    color: #ffffff;"
              "    font-size: 14px;"
              "    font-weight: bold;"
              "}"
              "#statsBadge, #zoomBadge {"
              "    color: #abb2bf;"
              "    background-color: #282c34;"
              "    border: 1px solid #3e4451;"
              "    border-radius: 10px;"
              "    padding: 2px 8px;"
              "    font-size: 11px;"
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
              "    background-color: %1;"
              "    color: #ffffff;"
              "    font-weight: bold;"
              "    border-radius: 6px;"
              "    padding: 6px 12px;"
              "    font-size: 12px;"
              "}"
              "#searchBox {"
              "    background-color: #282c34;"
              "    color: #abb2bf;"
              "    border: 1px solid #3e4451;"
              "    border-radius: 6px;"
              "    padding: 6px 10px;"
              "    font-size: 12px;"
              "}"
              "#searchBox:focus {"
              "    border: 1px solid %1;"
              "    color: #ffffff;"
              "}"
              "#fmtBtn, #lineBtn, #singleBtn, #zoomBtn {"
              "    background-color: #282c34;"
              "    color: #abb2bf;"
              "    border: 1px solid #3e4451;"
              "    border-radius: 4px;"
              "    padding: 2px 6px;"
              "    font-size: 11px;"
              "}"
              "#zoomBtn:hover {"
              "    background-color: #3e4451;"
              "    color: #ffffff;"
              "    border: 1px solid %1;"
              "}"
              "#fmtBtn:checked, #lineBtn:checked, #singleBtn:checked {"
              "    background-color: %1;"
              "    color: #ffffff;"
              "    border: 1px solid %1;"
              "    font-weight: bold;"
              "}"
              "#canvasScrollArea {"
              "    border: 1px solid #333842;"
              "    border-radius: 6px;"
              "    background-color: #16181d;"
              "}"
              "QScrollBar:vertical {"
              "    background: #1e222b;"
              "    width: 10px;"
              "    border-radius: 5px;"
              "    margin: 0px;"
              "}"
              "QScrollBar::handle:vertical {"
              "    background: #3e4451;"
              "    min-height: 24px;"
              "    border-radius: 5px;"
              "}"
              "QScrollBar::handle:vertical:hover {"
              "    background: %1;"
              "}"
              "QScrollBar:horizontal {"
              "    background: #1e222b;"
              "    height: 10px;"
              "    border-radius: 5px;"
              "    margin: 0px;"
              "}"
              "QScrollBar::handle:horizontal {"
              "    background: #3e4451;"
              "    min-width: 24px;"
              "    border-radius: 5px;"
              "}"
              "QScrollBar::handle:horizontal:hover {"
              "    background: %1;"
              "}"
              "QScrollBar::add-line, QScrollBar::sub-line {"
              "    border: none;"
              "    background: none;"
              "}"
              "#textEdit {"
              "    background-color: #21252b;"
              "    color: #d7dae0;"
              "    border: 1px solid #333842;"
              "    border-radius: 6px;"
              "    padding: 10px;"
              "    font-size: 13px;"
              "    line-height: 1.6;"
              "}"
              "#copyAllBtn {"
              "    background-color: %1;"
              "    color: #ffffff;"
              "    border: 1px solid rgba(255, 255, 255, 0.25);"
              "    border-radius: 6px;"
              "    padding: 6px 16px;"
              "    font-weight: bold;"
              "    font-size: 13px;"
              "}"
              "#copyAllBtn:hover {"
              "    background-color: %2;"
              "    border: 1px solid rgba(255, 255, 255, 0.6);"
              "    color: #ffffff;"
              "}"
              "#copyAllBtn:pressed {"
              "    background-color: %3;"
              "}"
              "#copySelBtn, #actionBtn, #doneBtn {"
              "    background-color: #282c34;"
              "    color: #d7dae0;"
              "    border: 1px solid #3e4451;"
              "    border-radius: 6px;"
              "    padding: 6px 12px;"
              "    font-size: 12px;"
              "    font-weight: 500;"
              "}"
              "#copySelBtn:hover, #actionBtn:hover, #doneBtn:hover {"
              "    background-color: #3e4451;"
              "    color: #ffffff;"
              "    border: 1px solid %1;"
              "}"
              "#copySelBtn:pressed, #actionBtn:pressed, #doneBtn:pressed {"
              "    background-color: #21252b;"
              "}")
        .arg(accentHex, accentHover, accentPressed));
}

void OcrWidget::updateZoomBadge(qreal factor)
{
    QString pctStr = QStringLiteral("%1%").arg(qRound(factor * 100));
    if (m_zoomBadge) {
        m_zoomBadge->setText(pctStr);
    }
    if (m_zoomBtn) {
        m_zoomBtn->setText(pctStr);
    }
}

void OcrWidget::showToast(const QString& message)
{
    m_toastLabel->setText(message);
    m_toastLabel->setVisible(true);
    m_toastTimer->start(2800);
}

void OcrWidget::copyAllText()
{
    QString txt = m_textEdit->toPlainText();
    if (!txt.isEmpty()) {
        QApplication::clipboard()->setText(txt);
        showToast(tr("All text copied to clipboard!"));
    }
}

void OcrWidget::copySelectedText()
{
    QString txt = m_textEdit->textCursor().selectedText();
    if (txt.isEmpty() && m_canvas) {
        txt = m_canvas->selectedText();
    }
    if (!txt.isEmpty()) {
        QApplication::clipboard()->setText(txt);
        showToast(tr("Selected text copied to clipboard!"));
    } else {
        copyAllText();
    }
}

void OcrWidget::searchWeb()
{
    QString txt = m_textEdit->textCursor().selectedText();
    if (txt.isEmpty() && m_canvas) {
        txt = m_canvas->selectedText();
    }
    if (txt.isEmpty()) {
        txt = m_textEdit->toPlainText();
    }
    txt = txt.trimmed();
    if (!txt.isEmpty()) {
        if (txt.startsWith(QStringLiteral("http://"), Qt::CaseInsensitive) ||
            txt.startsWith(QStringLiteral("https://"), Qt::CaseInsensitive)) {
            QDesktopServices::openUrl(QUrl(txt));
        } else {
            QUrl url(QStringLiteral("https://www.google.com/search?q=") +
                     QUrl::toPercentEncoding(txt));
            QDesktopServices::openUrl(url);
        }
    }
}

void OcrWidget::translateText()
{
    QString txt = m_textEdit->textCursor().selectedText();
    if (txt.isEmpty() && m_canvas) {
        txt = m_canvas->selectedText();
    }
    if (txt.isEmpty()) {
        txt = m_textEdit->toPlainText();
    }
    txt = txt.trimmed();
    if (!txt.isEmpty()) {
        QUrl url(
          QStringLiteral("https://translate.google.com/?sl=auto&tl=en&text=") +
          QUrl::toPercentEncoding(txt));
        QDesktopServices::openUrl(url);
    }
}

void OcrWidget::saveToFile()
{
    QString defaultName =
      QStringLiteral("ocr_") +
      QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss")) +
      QStringLiteral(".txt");
    QString fileName =
      QFileDialog::getSaveFileName(this,
                                   tr("Save OCR Text"),
                                   defaultName,
                                   tr("Text Files (*.txt);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << m_textEdit->toPlainText();
            file.close();
            showToast(tr("Saved to %1").arg(QFileInfo(fileName).fileName()));
        }
    }
}

void OcrWidget::pinScreenshot()
{
    close();
}

void OcrWidget::onSearchTextChanged(const QString& text)
{
    if (m_canvas) {
        m_canvas->setFilterQuery(text);
    }
}

QString OcrWidget::formatAsMarkdownTable() const
{
    if (m_lines.isEmpty()) {
        return QString();
    }

    QVector<QStringList> tableData;
    int maxCols = 0;

    for (const QString& line : m_lines) {
        QStringList cells;
        QString cleanLine = line.trimmed();
        if (cleanLine.contains(QLatin1Char('|'))) {
            cells = cleanLine.split(QLatin1Char('|'), Qt::SkipEmptyParts);
        } else if (cleanLine.contains(QLatin1Char('\t'))) {
            cells = cleanLine.split(QLatin1Char('\t'), Qt::SkipEmptyParts);
        } else {
            cells = cleanLine.split(QRegularExpression(QStringLiteral("\\s{2,}")), Qt::SkipEmptyParts);
        }

        for (int i = 0; i < cells.size(); ++i) {
            cells[i] = cells[i].trimmed();
        }

        if (cells.size() > maxCols) {
            maxCols = cells.size();
        }
        tableData.append(cells);
    }

    if (maxCols <= 1) {
        tableData.clear();
        maxCols = 0;
        for (const QString& line : m_lines) {
            QStringList cells = line.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
            if (cells.size() > maxCols) maxCols = cells.size();
            tableData.append(cells);
        }
    }

    if (maxCols == 0) maxCols = 1;

    QVector<int> colWidths(maxCols, 3);
    for (const auto& row : tableData) {
        for (int c = 0; c < row.size() && c < maxCols; ++c) {
            if (row[c].length() > colWidths[c]) {
                colWidths[c] = row[c].length();
            }
        }
    }

    QStringList resultLines;
    if (!tableData.isEmpty()) {
        QStringList headerCells;
        for (int c = 0; c < maxCols; ++c) {
            QString val = c < tableData[0].size() ? tableData[0][c] : QString();
            headerCells.append(val.leftJustified(colWidths[c]));
        }
        resultLines.append(QStringLiteral("| ") + headerCells.join(QStringLiteral(" | ")) + QStringLiteral(" |"));

        QStringList divCells;
        for (int c = 0; c < maxCols; ++c) {
            divCells.append(QString().fill(QLatin1Char('-'), qMax(3, colWidths[c])));
        }
        resultLines.append(QStringLiteral("| ") + divCells.join(QStringLiteral(" | ")) + QStringLiteral(" |"));

        for (int r = 1; r < tableData.size(); ++r) {
            QStringList dataCells;
            for (int c = 0; c < maxCols; ++c) {
                QString val = c < tableData[r].size() ? tableData[r][c] : QString();
                dataCells.append(val.leftJustified(colWidths[c]));
            }
            resultLines.append(QStringLiteral("| ") + dataCells.join(QStringLiteral(" | ")) + QStringLiteral(" |"));
        }
    }

    return resultLines.join(QStringLiteral("\n"));
}

QString OcrWidget::formatAsCodeBlock() const
{
    QString txt = m_fullText;
    QString lang;
    if (txt.contains("def ") || txt.contains("import ") || txt.contains("print(")) {
        lang = QStringLiteral("python");
    } else if (txt.contains("#include") || txt.contains("std::") || txt.contains("int main(")) {
        lang = QStringLiteral("cpp");
    } else if (txt.contains("function ") || txt.contains("const ") || txt.contains("let ") || txt.contains("console.log")) {
        lang = QStringLiteral("javascript");
    } else if (txt.contains("<html>") || txt.contains("<div>") || txt.contains("class=")) {
        lang = QStringLiteral("html");
    } else if (txt.contains("SELECT ") || txt.contains("FROM ") || txt.contains("WHERE ")) {
        lang = QStringLiteral("sql");
    } else if (txt.contains("curl ") || txt.contains("sudo ") || txt.contains("apt ")) {
        lang = QStringLiteral("bash");
    }

    return QStringLiteral("```") + lang + QStringLiteral("\n") + txt + QStringLiteral("\n```");
}

QString OcrWidget::formatAsLatex() const
{
    QString txt = m_fullText;
    bool looksLikeTable = false;
    for (const QString& line : m_lines) {
        if (line.contains(QLatin1Char('|')) || line.contains(QRegularExpression(QStringLiteral("\\s{2,}")))) {
            looksLikeTable = true;
            break;
        }
    }

    if (looksLikeTable && m_lines.size() > 1) {
        QStringList result;
        int maxCols = 0;
        QVector<QStringList> rows;
        for (const QString& line : m_lines) {
            QStringList cells = line.trimmed().split(QRegularExpression(QStringLiteral("\\s{2,}|\\||\t")), Qt::SkipEmptyParts);
            if (cells.size() > maxCols) maxCols = cells.size();
            rows.append(cells);
        }
        if (maxCols == 0) maxCols = 1;

        QString colSpec = QString().fill(QLatin1Char('c'), maxCols);
        result.append(QStringLiteral("\\begin{tabular}{|") + colSpec.split(QString(), Qt::SkipEmptyParts).join(QStringLiteral("|")) + QStringLiteral("|}"));
        result.append(QStringLiteral("\\hline"));
        for (const auto& r : rows) {
            QStringList padded;
            for (int c = 0; c < maxCols; ++c) {
                padded.append(c < r.size() ? r[c].trimmed() : QString());
            }
            result.append(padded.join(QStringLiteral(" & ")) + QStringLiteral(" \\\\ \\hline"));
        }
        result.append(QStringLiteral("\\end{tabular}"));
        return result.join(QStringLiteral("\n"));
    } else {
        QString mathTxt = txt;
        mathTxt.replace(QStringLiteral(" * "), QStringLiteral(" \\cdot "));
        mathTxt.replace(QStringLiteral(" <= "), QStringLiteral(" \\le "));
        mathTxt.replace(QStringLiteral(" >= "), QStringLiteral(" \\ge "));
        mathTxt.replace(QStringLiteral(" != "), QStringLiteral(" \\neq "));
        mathTxt.replace(QStringLiteral("alpha"), QStringLiteral("\\alpha"));
        mathTxt.replace(QStringLiteral("beta"), QStringLiteral("\\beta"));
        mathTxt.replace(QStringLiteral("gamma"), QStringLiteral("\\gamma"));
        mathTxt.replace(QStringLiteral("theta"), QStringLiteral("\\theta"));
        mathTxt.replace(QStringLiteral("lambda"), QStringLiteral("\\lambda"));
        mathTxt.replace(QStringLiteral("sigma"), QStringLiteral("\\sigma"));
        mathTxt.replace(QStringLiteral("pi"), QStringLiteral("\\pi"));
        mathTxt.replace(QStringLiteral("sqrt"), QStringLiteral("\\sqrt"));
        mathTxt.replace(QStringLiteral("sum"), QStringLiteral("\\sum"));
        mathTxt.replace(QStringLiteral("int"), QStringLiteral("\\int"));
        mathTxt.replace(QStringLiteral("inf"), QStringLiteral("\\infty"));

        return QStringLiteral("\\begin{equation}\n") + mathTxt + QStringLiteral("\n\\end{equation}");
    }
}

void OcrWidget::setFormatMode(int mode)
{
    m_currentFormatMode = mode;
    if (mode == 0) {
        m_textEdit->setPlainText(m_fullText);
    } else if (mode == 1) {
        m_textEdit->setPlainText(m_lines.join(QStringLiteral("\n")));
    } else if (mode == 2) {
        m_textEdit->setPlainText(m_words.join(QStringLiteral(" ")));
    } else if (mode == 3) {
        m_textEdit->setPlainText(formatAsMarkdownTable());
    } else if (mode == 4) {
        m_textEdit->setPlainText(formatAsCodeBlock());
    } else if (mode == 5) {
        m_textEdit->setPlainText(formatAsLatex());
    }
}

void OcrWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDraggingWindow = true;
        m_dragPosition =
          event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
}

void OcrWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDraggingWindow && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
    }
}

void OcrWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDraggingWindow = false;
    }
}

void OcrWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape ||
        (event->modifiers() & Qt::ControlModifier &&
         event->key() == Qt::Key_Q)) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}


