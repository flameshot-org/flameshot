// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2018 Alejandro Sirgo Rica & Contributors

#include "capturelauncher.h"
#include "./ui_capturelauncher.h"
#include "src/config/cacheutils.h"
#include "src/core/flameshot.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/utils/globalvalues.h"
#include "src/utils/screengrabber.h"
#include "src/utils/screenshotsaver.h"
#include "src/widgets/imagelabel.h"
#include <QGuiApplication>
#include <QMimeData>
#include <QScreen>

// https://github.com/KDE/spectacle/blob/941c1a517be82bed25d1254ebd735c29b0d2951c/src/Gui/KSWidget.cpp
// https://github.com/KDE/spectacle/blob/941c1a517be82bed25d1254ebd735c29b0d2951c/src/Gui/KSMainWindow.cpp

CaptureLauncher::CaptureLauncher(QDialog* parent)
  : QDialog(parent)
  , ui(new Ui::CaptureLauncher)
  , m_countdownTimer(new QTimer(this))
  , m_remainingSeconds(0)
{
    qApp->installEventFilter(this); // see eventFilter()
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    bool ok;

    ui->captureType->insertItem(
      1, tr("Rectangular Region"), CaptureRequest::GRAPHICAL_MODE);

#if defined(Q_OS_MACOS)
    // Following to MacOS philosophy (one application cannot be displayed on
    // more than one display)
    ui->captureType->insertItem(
      2, tr("Full Screen (Current Display)"), CaptureRequest::FULLSCREEN_MODE);
#else
    ui->captureType->insertItem(
      2, tr("Full Screen (All Monitors)"), CaptureRequest::FULLSCREEN_MODE);
    const QList<QScreen*> screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        QScreen* screen = screens[i];
        QRect geom = screen->geometry();
        QString monitorText = tr("Monitor %1: %2 (%3x%4)")
                                .arg(i + 1)
                                .arg(screen->name())
                                .arg(geom.width())
                                .arg(geom.height());
        ui->monitorSelection->addItem(monitorText, i);
    }
    // Select current screen by default
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    int currentIndex = screens.indexOf(currentScreen);
    if (currentIndex >= 0) {
        ui->monitorSelection->setCurrentIndex(currentIndex);
    }
#endif

#ifdef Q_OS_MACOS
    ui->monitorLabel->setVisible(false);
    ui->monitorSelection->setVisible(false);
#endif

    ui->delayTime->setSpecialValueText(tr("No Delay"));
    ui->launchButton->setFocus();

    ui->countdownLabel->setVisible(false);
    ui->countdownLabel->setStyleSheet(
      "QLabel { font-size: 24px; font-weight: bold;}");
    ui->countdownLabel->setAlignment(Qt::AlignCenter);

    // Connect countdown timer
    connect(m_countdownTimer,
            &QTimer::timeout,
            this,
            &CaptureLauncher::updateCountdown);

    // Function to add or remove plural to seconds
    connect(ui->delayTime,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,
            [this](int val) {
                QString suffix = val == 1 ? tr(" second") : tr(" seconds");
                this->ui->delayTime->setSuffix(suffix);
            });

    connect(ui->launchButton,
            &QPushButton::clicked,
            this,
            &CaptureLauncher::startCapture);

    connect(ui->captureType,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this]() {
                auto mode = static_cast<CaptureRequest::CaptureMode>(
                  ui->captureType->currentData().toInt());
                if (mode == CaptureRequest::CaptureMode::GRAPHICAL_MODE) {
                    ui->sizeLabel->show();
                    ui->screenshotX->show();
                    ui->screenshotY->show();
                    ui->screenshotWidth->show();
                    ui->screenshotHeight->show();
                } else {
                    ui->sizeLabel->hide();
                    ui->screenshotX->hide();
                    ui->screenshotY->hide();
                    ui->screenshotWidth->hide();
                    ui->screenshotHeight->hide();
                }
            });

    auto lastRegion = getLastRegion();
    ui->screenshotX->setText(QString::number(lastRegion.x()));
    ui->screenshotY->setText(QString::number(lastRegion.y()));
    ui->screenshotWidth->setText(QString::number(lastRegion.width()));
    ui->screenshotHeight->setText(QString::number(lastRegion.height()));

    show();
    // Call show() first, otherwise the correct geometry cannot be fetched
    // for centering the window on the screen
    QRect position = frameGeometry();
    QScreen* screen = QGuiAppCurrentScreen().currentScreen();
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());
}

// HACK:
// https://github.com/KDE/spectacle/blob/fa1e780b8bf3df3ac36c410b9ece4ace041f401b/src/Gui/KSMainWindow.cpp#L70
void CaptureLauncher::startCapture()
{
    ui->launchButton->setEnabled(false);

    int delaySeconds = ui->delayTime->value();

    if (delaySeconds > 0) {
        m_remainingSeconds = delaySeconds;
        ui->countdownLabel->setVisible(true);
        ui->countdownLabel->setText(QString::number(m_remainingSeconds));
        m_countdownTimer->start(1000);
    } else {
        hide();
    }

    auto const additionalDelayToHideUI = 600;
    auto const secondsToMilliseconds = 1000;
    auto mode = static_cast<CaptureRequest::CaptureMode>(
      ui->captureType->currentData().toInt());
    CaptureRequest req(
      mode, additionalDelayToHideUI + delaySeconds * secondsToMilliseconds);

    if (mode == CaptureRequest::CaptureMode::GRAPHICAL_MODE) {
        req.setInitialSelection(QRect(ui->screenshotX->text().toInt(),
                                      ui->screenshotY->text().toInt(),
                                      ui->screenshotWidth->text().toInt(),
                                      ui->screenshotHeight->text().toInt()));
    }

#ifndef Q_OS_MACOS
    int selectedMonitor = ui->monitorSelection->currentData().toInt();
    req.setSelectedMonitor(selectedMonitor);
#else
    req.setSelectedMonitor(-1);
#endif

    connectCaptureSlots();
    Flameshot::instance()->requestCapture(req);
}

void CaptureLauncher::connectCaptureSlots() const
{
    connect(Flameshot::instance(),
            &Flameshot::captureTaken,
            this,
            &CaptureLauncher::onCaptureTaken);
    connect(Flameshot::instance(),
            &Flameshot::captureFailed,
            this,
            &CaptureLauncher::onCaptureFailed);
}

void CaptureLauncher::disconnectCaptureSlots() const
{
    // Hack for MacOS
    // for some strange reasons MacOS sends multiple "captureTaken" signals
    // (random number, usually from 1 up to 20).
    // So now it enables signal on "Capture new screenshot" button and disables
    // on first success of fail.
    disconnect(Flameshot::instance(),
               &Flameshot::captureTaken,
               this,
               &CaptureLauncher::onCaptureTaken);
    disconnect(Flameshot::instance(),
               &Flameshot::captureFailed,
               this,
               &CaptureLauncher::onCaptureFailed);
}

void CaptureLauncher::onCaptureTaken(QPixmap const& screenshot)
{
    // MacOS specific, more details in the function disconnectCaptureSlots()
    disconnectCaptureSlots();
    m_countdownTimer->stop();
    ui->countdownLabel->setVisible(false);

    auto mode = static_cast<CaptureRequest::CaptureMode>(
      ui->captureType->currentData().toInt());

    if (mode == CaptureRequest::FULLSCREEN_MODE) {
        saveToFilesystemGUI(screenshot);
    }
    ui->launchButton->setEnabled(true);
}

void CaptureLauncher::onCaptureFailed()
{
    // MacOS specific, more details in the function disconnectCaptureSlots()
    disconnectCaptureSlots();
    m_countdownTimer->stop();
    ui->countdownLabel->setVisible(false);
    show();
    ui->launchButton->setEnabled(true);
}

void CaptureLauncher::updateCountdown()
{
    m_remainingSeconds--;

    if (m_remainingSeconds > 0) {
        ui->countdownLabel->setText(QString::number(m_remainingSeconds));
    } else {
        m_countdownTimer->stop();
        ui->countdownLabel->setVisible(false);
        hide();
    }
}

CaptureLauncher::~CaptureLauncher()
{
    delete ui;
}
