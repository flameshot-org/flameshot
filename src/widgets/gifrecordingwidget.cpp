// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#include "gifrecordingwidget.h"

#include "utils/confighandler.h"
#include "utils/pathinfo.h"
#include "utils/x11rootgrab.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QColorDialog>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPoint>
#include <QPushButton>
#include <QScreen>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

GifRecordingWidget::GifRecordingWidget(const QRect& region,
                                       int durationSeconds,
                                       QWidget* parent)
  : QWidget(parent)
  , m_durationSeconds(durationSeconds)
  , m_region(region)
  , m_color(ConfigHandler().drawColor())
{
    setWindowTitle(tr("GIF recording"));
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::StrongFocus);

    setStyleSheet(QStringLiteral(
      "QWidget { background-color: #141414;"
      "          border: 1px solid #3a3a3a;"
      "          border-radius: 8px; }"
      "QLabel { color: white; background: transparent; }"
      "QLabel#recordingDot { color: #ff3b30; font-size: 18px; }"
      "QLabel#hintLabel { color: #b0b0b0; font-size: 11px; }"
      "QPushButton { background-color: #3a3a3a; color: white;"
      "              border: none; border-radius: 4px; padding: 4px 10px; }"
      "QPushButton:hover { background-color: #4a4a4a; }"
      "QPushButton#stopButton { background-color: #c0392b; }"
      "QPushButton#stopButton:hover { background-color: #d64535; }"
      "QToolButton { background-color: #3a3a3a; border: 1px solid #555;"
      "              border-radius: 4px; padding: 2px; }"
      "QToolButton:hover { background-color: #4a4a4a; }"
      "QToolButton:checked { background-color: #c0392b;"
      "                     border: 1px solid #ff8a80; }"
      "QSpinBox { background-color: #3a3a3a; color: white;"
      "           border: 1px solid #555; border-radius: 4px; }"));

    auto* dot = new QLabel(QStringLiteral("\u25CF"), this);
    dot->setObjectName(QStringLiteral("recordingDot"));

    m_statusLabel = new QLabel(this);
    m_statusLabel->setMinimumWidth(150);
    m_statusLabel->setText(tr("Recording GIF... 00:00"));

    auto* stopButton = new QPushButton(tr("Stop"), this);
    stopButton->setObjectName(QStringLiteral("stopButton"));
    stopButton->setCursor(Qt::PointingHandCursor);

    auto* cancelButton = new QPushButton(tr("Cancel"), this);
    cancelButton->setCursor(Qt::PointingHandCursor);

    m_hintLabel = new QLabel(this);
    m_hintLabel->setObjectName(QStringLiteral("hintLabel"));

    m_toolsRow = new QWidget(this);
    auto* toolsLayout = new QHBoxLayout(m_toolsRow);
    toolsLayout->setContentsMargins(0, 0, 0, 0);
    toolsLayout->setSpacing(4);

    m_toolGroup = new QButtonGroup(this);
    m_toolGroup->setExclusive(true);

    auto addTool = [this, toolsLayout](GifDrawingTool tool,
                                       const QString& iconName,
                                       const QString& label,
                                       const QString& tip) {
        auto* button = new QToolButton(m_toolsRow);
        button->setCheckable(true);
        button->setFixedSize(28, 28);
        button->setToolTip(tip);
        const QIcon icon(PathInfo::whiteIconPath() + iconName);
        if (!icon.isNull()) {
            button->setIcon(icon);
        } else {
            button->setText(label);
        }
        m_toolGroup->addButton(button, static_cast<int>(tool));
        toolsLayout->addWidget(button);
    };

    addTool(GifDrawingTool::Pencil, QStringLiteral("pencil.svg"), tr("P"),
            tr("Pencil"));
    addTool(GifDrawingTool::Line, QStringLiteral("line.svg"), tr("L"),
            tr("Line"));
    addTool(GifDrawingTool::Arrow, QStringLiteral("arrow-bottom-left.svg"),
            tr("A"), tr("Arrow"));
    addTool(GifDrawingTool::Rectangle, QStringLiteral("square-outline.svg"),
            tr("R"), tr("Rectangle"));
    addTool(GifDrawingTool::Ellipse, QStringLiteral("circle-outline.svg"),
            tr("E"), tr("Ellipse"));
    addTool(GifDrawingTool::Text, QStringLiteral("text.svg"), tr("T"),
            tr("Text"));
    addTool(GifDrawingTool::Number,
            QStringLiteral("circlecount-outline.svg"),
            tr("1"),
            tr("Number"));

    m_colorButton = new QPushButton(m_toolsRow);
    m_colorButton->setFixedSize(28, 28);
    m_colorButton->setToolTip(tr("Drawing color"));
    updateColorButton();
    toolsLayout->addWidget(m_colorButton);

    m_widthSpinBox = new QSpinBox(m_toolsRow);
    m_widthSpinBox->setRange(1, 30);
    m_widthSpinBox->setValue(ConfigHandler().drawThickness());
    m_widthSpinBox->setFixedWidth(48);
    m_widthSpinBox->setToolTip(tr("Line width"));
    toolsLayout->addWidget(m_widthSpinBox);

    auto* fpsLabel = new QLabel(tr("FPS"), m_toolsRow);
    toolsLayout->addWidget(fpsLabel);
    m_fpsSpinBox = new QSpinBox(m_toolsRow);
    m_fpsSpinBox->setRange(1, 50);
    m_fpsSpinBox->setValue(ConfigHandler().gifFps());
    m_fpsSpinBox->setFixedWidth(48);
    m_fpsSpinBox->setToolTip(tr("Frames per second"));
    toolsLayout->addWidget(m_fpsSpinBox);

    auto* undoButton = new QPushButton(tr("Undo"), m_toolsRow);
    undoButton->setCursor(Qt::PointingHandCursor);
    auto* clearButton = new QPushButton(tr("Clear"), m_toolsRow);
    clearButton->setCursor(Qt::PointingHandCursor);
    toolsLayout->addWidget(undoButton);
    toolsLayout->addWidget(clearButton);
    toolsLayout->addStretch();
    m_toolsRow->hide();

    auto* buttons = new QHBoxLayout;
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setSpacing(6);
    buttons->addWidget(stopButton);
    buttons->addWidget(cancelButton);

    auto* topRow = new QHBoxLayout;
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->setSpacing(8);
    topRow->addWidget(dot);
    topRow->addWidget(m_statusLabel, 1);
    topRow->addLayout(buttons);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(2);
    layout->addLayout(topRow);
    layout->addWidget(m_toolsRow);

    m_textEdit = new QLineEdit(this);
    m_textEdit->setPlaceholderText(tr("Text to place"));
    m_textEdit->setToolTip(tr("Type text, then click the screen to place it"));
    m_textEdit->hide();
    layout->addWidget(m_textEdit);

    layout->addWidget(m_hintLabel);

    connect(stopButton,
            &QPushButton::clicked,
            this,
            &GifRecordingWidget::stopRequested);
    connect(cancelButton,
            &QPushButton::clicked,
            this,
            &GifRecordingWidget::cancelRequested);
    connect(m_toolGroup,
            &QButtonGroup::idClicked,
            this,
            [this](int id) {
                const auto tool = static_cast<GifDrawingTool>(id);
                if (m_textEdit && tool != GifDrawingTool::Text) {
                    m_textEdit->hide();
                }
                // Let the overlay switch its input mode first, then focus the
                // panel. Otherwise raising/shaping the overlay can steal the
                // focus that was just given to the text field.
                emit toolSelected(tool);
                if (tool == GifDrawingTool::Text) {
                    focusTextInput();
                }
            });
    connect(m_textEdit,
            &QLineEdit::returnPressed,
            this,
            &GifRecordingWidget::textSubmitted);
    connect(m_colorButton, &QPushButton::clicked, this, [this]() {
        const QColor color =
          QColorDialog::getColor(m_color, this, tr("Drawing color"));
        if (color.isValid()) {
            m_color = color;
            updateColorButton();
            emit colorChanged(color);
        }
    });
    connect(m_widthSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            [this](int value) { emit widthChanged(value); });
    connect(m_fpsSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            [this](int value) {
                emit fpsChanged(value);
            });
    connect(undoButton,
            &QPushButton::clicked,
            this,
            &GifRecordingWidget::undoRequested);
    connect(clearButton,
            &QPushButton::clicked,
            this,
            &GifRecordingWidget::clearRequested);

    auto* undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut,
            &QShortcut::activated,
            this,
            &GifRecordingWidget::undoRequested);

    updateHint();
    adjustSize();
    placeNearRegion(m_region);
}

void GifRecordingWidget::setElapsed(qint64 elapsedMs, int frameCount)
{
    QString text = tr("%1  %2 frames")
                     .arg(formatDuration(elapsedMs))
                     .arg(frameCount);
    if (m_durationSeconds > 0) {
        text += QStringLiteral(" / ") +
                formatDuration(qint64(m_durationSeconds) * 1000);
    }
    m_statusLabel->setText(text);
}

void GifRecordingWidget::setInteractive(bool interactive)
{
    m_interactive = interactive;
    if (m_toolsRow) {
        m_toolsRow->setVisible(interactive);
    }
    if (m_textEdit && !interactive) {
        m_textEdit->hide();
    }
    updateHint();
    adjustSize();
    placeNearRegion(m_region);
}

void GifRecordingWidget::clearToolSelection()
{
    if (!m_toolGroup) {
        return;
    }
    const bool exclusive = m_toolGroup->exclusive();
    m_toolGroup->setExclusive(false);
    if (auto* button = m_toolGroup->checkedButton()) {
        button->setChecked(false);
    }
    m_toolGroup->setExclusive(exclusive);
    if (m_textEdit) {
        m_textEdit->hide();
    }
}

QString GifRecordingWidget::textInput() const
{
    return m_textEdit ? m_textEdit->text() : QString();
}

void GifRecordingWidget::focusTextInput()
{
    if (!m_textEdit) {
        return;
    }

    m_textEdit->show();
    adjustSize();
    placeNearRegion(m_region);

    // The drawing overlay never accepts keyboard focus, but the panel must be
    // the active window for the user to be able to type. Do it again on the
    // next event-loop iteration in case the window manager applies the
    // activation asynchronously.
    activateWindow();
    raise();
    m_textEdit->setFocus(Qt::OtherFocusReason);
    m_textEdit->selectAll();

    QTimer::singleShot(50, m_textEdit, [this]() {
        activateWindow();
        raise();
        m_textEdit->setFocus(Qt::OtherFocusReason);
        m_textEdit->selectAll();
    });
}

void GifRecordingWidget::setFps(int fps)
{
    if (!m_fpsSpinBox) {
        return;
    }
    const QSignalBlocker blocker(m_fpsSpinBox);
    m_fpsSpinBox->setValue(
      qBound(m_fpsSpinBox->minimum(), fps, m_fpsSpinBox->maximum()));
}

void GifRecordingWidget::updateColorButton()
{
    if (!m_colorButton) {
        return;
    }
    m_colorButton->setStyleSheet(
      QStringLiteral("QPushButton { background-color: %1;"
                     "              border: 2px solid #ffffff;"
                     "              border-radius: 4px; }")
        .arg(m_color.name()));
}

void GifRecordingWidget::updateHint()
{
    if (!m_hintLabel) {
        return;
    }
    if (m_interactive) {
        m_hintLabel->setText(
          tr("Pick a tool to draw. Without a tool, clicks pass through to the "
             "desktop."));
    } else if (m_durationSeconds > 0) {
        m_hintLabel->setText(
          tr("Stops automatically after %1 s").arg(m_durationSeconds));
    } else {
        m_hintLabel->setText(tr("Click Stop when you are done"));
    }
}

QString GifRecordingWidget::formatDuration(qint64 milliseconds) const
{
    const qint64 totalSeconds = milliseconds / 1000;
    return QStringLiteral("%1:%2")
      .arg(totalSeconds / 60, 2, 10, QLatin1Char('0'))
      .arg(totalSeconds % 60, 2, 10, QLatin1Char('0'));
}

QRect GifRecordingWidget::fitOutsideRegion(const QRect& region)
{
    QScreen* screen = QGuiApplication::screenAt(region.center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    QRect available;
    if (screen) {
        available = screen->availableGeometry();
    } else {
        // XRandR may expose no monitors on virtual/remote displays.
        available = x11RootGeometry();
    }
    if (available.isEmpty()) {
        m_region = region;
        return region;
    }

    // Reserve room for the text field as well: it is shown on demand and makes
    // the panel taller. The reservation keeps that later growth outside the
    // recorded region too.
    const QSize panelSize = size();
    const int extraHeight = m_interactive ? 40 : 0;
    m_reservedSize = QSize(panelSize.width(), panelSize.height() + extraHeight);

    if (placeNearRegion(region)) {
        return region;
    }

    // The selected region leaves no room for the panel. Shrink it on the side
    // that loses the least area, then place the panel outside the result.
    constexpr int gap = 8;
    QList<QRect> candidates;

    {
        QRect candidate = region;
        candidate.setBottom(qMin(
          region.bottom(),
          available.bottom() - m_reservedSize.height() - gap));
        if (candidate.height() >= 1) {
            candidates.append(candidate);
        }
    }
    {
        QRect candidate = region;
        candidate.setTop(qMax(
          region.top(), available.top() + m_reservedSize.height() + gap));
        if (candidate.height() >= 1) {
            candidates.append(candidate);
        }
    }
    {
        QRect candidate = region;
        candidate.setRight(qMin(
          region.right(),
          available.right() - m_reservedSize.width() - gap));
        if (candidate.width() >= 1) {
            candidates.append(candidate);
        }
    }
    {
        QRect candidate = region;
        candidate.setLeft(qMax(
          region.left(), available.left() + m_reservedSize.width() + gap));
        if (candidate.width() >= 1) {
            candidates.append(candidate);
        }
    }

    std::sort(candidates.begin(),
              candidates.end(),
              [](const QRect& a, const QRect& b) {
                  return a.width() * a.height() > b.width() * b.height();
              });
    for (const QRect& candidate : candidates) {
        if (placeNearRegion(candidate)) {
            return candidate;
        }
    }

    // Extremely small screens can leave no room at all. Keep the selected
    // region; this is the only case where the panel may overlap it.
    placeNearRegion(region);
    return region;
}

bool GifRecordingWidget::placeNearRegion(const QRect& region)
{
    m_region = region;

    QScreen* screen = QGuiApplication::screenAt(region.center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    QRect available;
    if (screen) {
        available = screen->availableGeometry();
    } else {
        // XRandR may expose no monitors on virtual/remote displays.
        available = x11RootGeometry();
    }
    if (available.isEmpty()) {
        return false;
    }

    const QSize panelSize = m_reservedSize.isValid() ? m_reservedSize : size();
    constexpr int gap = 8;

    const QList<QPoint> candidates = {
        QPoint(region.center().x() - panelSize.width() / 2,
               region.bottom() + gap),
        QPoint(region.center().x() - panelSize.width() / 2,
               region.top() - panelSize.height() - gap),
        QPoint(region.right() + gap,
               region.center().y() - panelSize.height() / 2),
        QPoint(region.left() - panelSize.width() - gap,
               region.center().y() - panelSize.height() / 2),
    };

    for (const QPoint& point : candidates) {
        const QRect candidate(point, panelSize);
        if (available.contains(candidate) && !candidate.intersects(region)) {
            move(candidate.topLeft());
            return true;
        }
    }

    // If the region covers a whole monitor, try another monitor that is not
    // being recorded, so the panel does not end up in the GIF.
    for (QScreen* otherScreen : QGuiApplication::screens()) {
        if (otherScreen == screen) {
            continue;
        }
        const QRect otherAvailable = otherScreen->availableGeometry();
        if (otherAvailable.intersects(region)) {
            continue;
        }
        const QRect candidate(
          QPoint(otherAvailable.right() - panelSize.width() + 1 - gap,
                 otherAvailable.bottom() - panelSize.height() + 1 - gap),
          panelSize);
        if (otherAvailable.contains(candidate)) {
            move(candidate.topLeft());
            return true;
        }
    }

    // The region covers the whole available area: keep the panel inside it,
    // but as far away from the centre as possible.
    QRect fallback(region.bottomRight() -
                     QPoint(panelSize.width(), panelSize.height()) +
                     QPoint(1 - gap, 1 - gap),
                   panelSize);
    fallback.moveLeft(qBound(available.left(),
                             fallback.left(),
                             available.right() - panelSize.width() + 1));
    fallback.moveTop(qBound(available.top(),
                            fallback.top(),
                            available.bottom() - panelSize.height() + 1));
    move(fallback.topLeft());
    return false;
}

void GifRecordingWidget::keyPressEvent(QKeyEvent* event)
{
    // Do not treat Enter/Space as Stop while the user edits a spin box or a
    // text field inside the panel.
    QWidget* focus = QApplication::focusWidget();
    if (focus && focus != this &&
        (qobject_cast<QSpinBox*>(focus) || qobject_cast<QLineEdit*>(focus))) {
        QWidget::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        emit cancelRequested();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Space) {
        emit stopRequested();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void GifRecordingWidget::closeEvent(QCloseEvent* event)
{
    if (m_interactive) {
        const auto reply = QMessageBox::question(
          this,
          tr("GIF recording"),
          tr("Stop and save the recording?"),
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
          QMessageBox::Yes);
        if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if (reply == QMessageBox::Yes) {
            emit stopRequested();
        } else {
            emit cancelRequested();
        }
        event->accept();
        return;
    }

    // Closing the panel (for example with Alt+F4) aborts the recording instead
    // of leaving it running without a way to stop it.
    emit cancelRequested();
    QWidget::closeEvent(event);
}
