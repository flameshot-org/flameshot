// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2018 Alejandro Sirgo Rica & Contributors

#include "capturelauncher.h"
#include "./ui_capturelauncher.h"
#include "src/core/flameshot.h"
#include "src/utils/globalvalues.h"
#include "src/utils/screengrabber.h"
#include "src/utils/screenshotsaver.h"
#include "src/widgets/imagelabel.h"
#include <QMimeData>

// https://github.com/KDE/spectacle/blob/941c1a517be82bed25d1254ebd735c29b0d2951c/src/Gui/KSWidget.cpp
// https://github.com/KDE/spectacle/blob/941c1a517be82bed25d1254ebd735c29b0d2951c/src/Gui/KSMainWindow.cpp

CaptureLauncher::CaptureLauncher(QDialog* parent)
  : QDialog(parent)
  , ui(new Ui::CaptureLauncher)
{
    qApp->installEventFilter(this); // see eventFilter()
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    bool ok;

    ui->imagePreview->setScreenshot(ScreenGrabber().grabEntireDesktop(ok));
    ui->imagePreview->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Expanding);

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
#endif

    ui->delayTime->setSpecialValueText(tr("No Delay"));
    ui->launchButton->setFocus();

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

    show();
}

// HACK:
// https://github.com/KDE/spectacle/blob/fa1e780b8bf3df3ac36c410b9ece4ace041f401b/src/Gui/KSMainWindow.cpp#L70
void CaptureLauncher::startCapture()
{
    ui->launchButton->setEnabled(false);
    hide();

    auto const additionalDelayToHideUI = 600;
    auto const secondsToMilliseconds = 1000;
    auto mode = static_cast<CaptureRequest::CaptureMode>(
      ui->captureType->currentData().toInt());
    CaptureRequest req(mode,
                       additionalDelayToHideUI +
                         ui->delayTime->value() * secondsToMilliseconds);
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

void CaptureLauncher::onCaptureTaken(QPixmap screenshot)
{
    // MacOS specific, more details in the function disconnectCaptureSlots()
    disconnectCaptureSlots();

    ui->imagePreview->setScreenshot(screenshot);
    show();

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
    show();
    ui->launchButton->setEnabled(true);
}

CaptureLauncher::~CaptureLauncher()
{
    delete ui;
}
