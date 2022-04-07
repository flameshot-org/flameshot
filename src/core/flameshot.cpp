// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "flameshot.h"
#include "flameshotdaemon.h"

#if defined(Q_OS_MACOS)
#include "external/QHotkey/QHotkey"
#endif

#include "abstractlogger.h"
#include "screenshotsaver.h"
#include "src/config/configresolver.h"
#include "src/config/configwindow.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/tools/imgupload/imguploadermanager.h"
#include "src/tools/imgupload/storages/imguploaderbase.h"
#include "src/utils/confighandler.h"
#include "src/utils/screengrabber.h"
#include "src/widgets/capture/capturewidget.h"
#include "src/widgets/capturelauncher.h"
#include "src/widgets/imguploaddialog.h"
#include "src/widgets/infowindow.h"
#include "src/widgets/uploadhistory.h"
#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFile>
#include <QMessageBox>
#include <QThread>
#include <QTimer>
#include <QVersionNumber>

#if defined(Q_OS_MACOS)
#include <QScreen>
#endif

Flameshot::Flameshot()
  : m_captureWindow(nullptr)
#if defined(Q_OS_MACOS)
  , m_HotkeyScreenshotCapture(nullptr)
  , m_HotkeyScreenshotHistory(nullptr)
#endif
{
    QString StyleSheet = CaptureButton::globalStyleSheet();
    qApp->setStyleSheet(StyleSheet);

#if defined(Q_OS_MACOS)
    // Try to take a test screenshot, MacOS will request a "Screen Recording"
    // permissions on the first run. Otherwise it will be hidden under the
    // CaptureWidget
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    currentScreen->grabWindow(QApplication::desktop()->winId(), 0, 0, 1, 1);

    // set global shortcuts for MacOS
    m_HotkeyScreenshotCapture = new QHotkey(
      QKeySequence(ConfigHandler().shortcut("TAKE_SCREENSHOT")), true, this);
    QObject::connect(m_HotkeyScreenshotCapture,
                     &QHotkey::activated,
                     qApp,
                     [this]() { gui(); });
    m_HotkeyScreenshotHistory = new QHotkey(
      QKeySequence(ConfigHandler().shortcut("SCREENSHOT_HISTORY")), true, this);
    QObject::connect(m_HotkeyScreenshotHistory,
                     &QHotkey::activated,
                     qApp,
                     [this]() { history(); });
#endif
}

Flameshot* Flameshot::instance()
{
    static Flameshot c;
    return &c;
}

CaptureWidget* Flameshot::gui(const CaptureRequest& req)
{
    if (!resolveAnyConfigErrors()) {
        return nullptr;
    }

#if defined(Q_OS_MACOS)
    // This is required on MacOS because of Mission Control. If you'll switch to
    // another Desktop you cannot take a new screenshot from the tray, you have
    // to switch back to the Flameshot Desktop manually. It is not obvious and a
    // large number of users are confused and report a bug.
    if (m_captureWindow != nullptr) {
        m_captureWindow->close();
        delete m_captureWindow;
        m_captureWindow = nullptr;
    }
#endif

    if (nullptr == m_captureWindow) {
        // TODO is this unnecessary now?
        int timeout = 5000; // 5 seconds
        const int delay = 100;
        QWidget* modalWidget = nullptr;
        for (; timeout >= 0; timeout -= delay) {
            modalWidget = qApp->activeModalWidget();
            if (nullptr == modalWidget) {
                break;
            }
            modalWidget->close();
            modalWidget->deleteLater();
            QThread::msleep(delay);
        }
        if (0 == timeout) {
            QMessageBox::warning(
              nullptr, tr("Error"), tr("Unable to close active modal widgets"));
            return nullptr;
        }

        m_captureWindow = new CaptureWidget(req);
        // m_captureWindow = new CaptureWidget(req, false); //
        // debug

#ifdef Q_OS_WIN
        m_captureWindow->show();
#elif defined(Q_OS_MACOS)
        // In "Emulate fullscreen mode"
        m_captureWindow->showFullScreen();
        m_captureWindow->activateWindow();
        m_captureWindow->raise();
#else
        m_captureWindow->showFullScreen();
//        m_captureWindow->show(); // For CaptureWidget Debugging under Linux
#endif
        return m_captureWindow;
    } else {
        emit captureFailed();
        return nullptr;
    }
}

void Flameshot::screen(CaptureRequest req, const int screenNumber)
{
    if (!resolveAnyConfigErrors())
        return;

    bool ok = true;
    QScreen* screen;

    if (screenNumber < 0) {
        QPoint globalCursorPos = QCursor::pos();
#if QT_VERSION > QT_VERSION_CHECK(5, 10, 0)
        screen = qApp->screenAt(globalCursorPos);
#else
        screen =
          qApp->screens()[qApp->desktop()->screenNumber(globalCursorPos)];
#endif
    } else if (screenNumber >= qApp->screens().count()) {
        AbstractLogger() << QObject::tr(
          "Requested screen exceeds screen count");
        emit captureFailed();
        return;
    } else {
        screen = qApp->screens()[screenNumber];
    }
    QPixmap p(ScreenGrabber().grabScreen(screen, ok));
    if (ok) {
        QRect geometry = ScreenGrabber().screenGeometry(screen);
        QRect region = req.initialSelection();
        if (region.isNull()) {
            region = ScreenGrabber().screenGeometry(screen);
        } else {
            QRect screenGeom = ScreenGrabber().screenGeometry(screen);
            screenGeom.moveTopLeft({ 0, 0 });
            region = region.intersected(screenGeom);
            p = p.copy(region);
        }
        if (req.tasks() & CaptureRequest::PIN) {
            // change geometry for pin task
            req.addPinTask(region);
        }
        exportCapture(p, geometry, req);
    } else {
        emit captureFailed();
    }
}

void Flameshot::full(const CaptureRequest& req)
{
    if (!resolveAnyConfigErrors())
        return;

    bool ok = true;
    QPixmap p(ScreenGrabber().grabEntireDesktop(ok));
    QRect region = req.initialSelection();
    if (!region.isNull()) {
        p = p.copy(region);
    }
    if (ok) {
        QRect selection; // `flameshot full` does not support --selection
        exportCapture(p, selection, req);
    } else {
        emit captureFailed();
    }
}

void Flameshot::launcher()
{
    if (!resolveAnyConfigErrors())
        return;

    if (!m_launcherWindow) {
        m_launcherWindow = new CaptureLauncher();
    }
    m_launcherWindow->show();
#if defined(Q_OS_MACOS)
    m_launcherWindow->activateWindow();
    m_launcherWindow->raise();
#endif
}

void Flameshot::config()
{
    if (!resolveAnyConfigErrors())
        return;

    if (!m_configWindow) {
        m_configWindow = new ConfigWindow();
        m_configWindow->show();
#if defined(Q_OS_MACOS)
        m_configWindow->activateWindow();
        m_configWindow->raise();
#endif
    }
}

void Flameshot::info()
{
    if (!m_infoWindow) {
        m_infoWindow = new InfoWindow();
#if defined(Q_OS_MACOS)
        m_infoWindow->activateWindow();
        m_infoWindow->raise();
#endif
    }
}

void Flameshot::history()
{
    static UploadHistory* historyWidget = nullptr;
    if (historyWidget == nullptr) {
        historyWidget = new UploadHistory;
        historyWidget->loadHistory();
        connect(historyWidget, &QObject::destroyed, this, []() {
            historyWidget = nullptr;
        });
    }
    historyWidget->show();

#if defined(Q_OS_MACOS)
    historyWidget->activateWindow();
    historyWidget->raise();
#endif
}

QVersionNumber Flameshot::getVersion()
{
    return QVersionNumber::fromString(
      QStringLiteral(APP_VERSION).replace("v", ""));
}

void Flameshot::setOrigin(Origin origin)
{
    m_origin = origin;
}

Flameshot::Origin Flameshot::origin()
{
    return m_origin;
}

/**
 * @brief Prompt the user to resolve config errors if necessary.
 * @return Whether errors were resolved.
 */
bool Flameshot::resolveAnyConfigErrors()
{
    bool resolved = true;
    ConfigHandler config;
    if (!config.checkUnrecognizedSettings() || !config.checkSemantics()) {
        auto* resolver = new ConfigResolver();
        QObject::connect(
          resolver, &ConfigResolver::rejected, [resolver, &resolved]() {
              resolved = false;
              resolver->deleteLater();
              if (origin() == CLI) {
                  exit(1);
              }
          });
        QObject::connect(
          resolver, &ConfigResolver::accepted, [resolver, &resolved]() {
              resolved = true;
              resolver->close();
              resolver->deleteLater();
              // Ensure that the dialog is closed before starting capture
              qApp->processEvents();
          });
        resolver->exec();
        qApp->processEvents();
    }
    return resolved;
}

void Flameshot::requestCapture(const CaptureRequest& request)
{
    if (!resolveAnyConfigErrors()) {
        return;
    }

    switch (request.captureMode()) {
        case CaptureRequest::FULLSCREEN_MODE:
            QTimer::singleShot(request.delay(),
                               [this, request] { full(request); });
            break;
        case CaptureRequest::SCREEN_MODE: {
            int&& number = request.data().toInt();
            QTimer::singleShot(request.delay(), [this, request, number]() {
                screen(request, number);
            });
            break;
        }
        case CaptureRequest::GRAPHICAL_MODE: {
            QTimer::singleShot(
              request.delay(), this, [this, request]() { gui(request); });
            break;
        }
        default:
            emit captureFailed();
            break;
    }
}

void Flameshot::exportCapture(QPixmap capture,
                              QRect& selection,
                              const CaptureRequest& req)
{
    using CR = CaptureRequest;
    int tasks = req.tasks(), mode = req.captureMode();
    QString path = req.path();

    if (tasks & CR::PRINT_GEOMETRY) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        QTextStream(stdout)
          << selection.width() << "x" << selection.height() << "+"
          << selection.x() << "+" << selection.y() << "\n";
    }

    if (tasks & CR::PRINT_RAW) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        capture.save(&buffer, "PNG");
        QFile file;
        file.open(stdout, QIODevice::WriteOnly);

        file.write(byteArray);
        file.close();
    }

    if (tasks & CR::SAVE) {
        if (req.path().isEmpty()) {
            saveToFilesystemGUI(capture);
        } else {
            saveToFilesystem(capture, path);
        }
    }

    if (tasks & CR::COPY) {
        FlameshotDaemon::copyToClipboard(capture);
    }

    if (tasks & CR::PIN) {
        FlameshotDaemon::createPin(capture, selection);
        if (mode == CR::SCREEN_MODE || mode == CR::FULLSCREEN_MODE) {
            AbstractLogger::info()
              << QObject::tr("Full screen screenshot pinned to screen");
        }
    }

    if (tasks & CR::UPLOAD) {
        if (!ConfigHandler().uploadWithoutConfirmation()) {
            auto* dialog = new ImgUploadDialog();
            if (dialog->exec() == QDialog::Rejected) {
                return;
            }
        }

        ImgUploaderBase* widget = ImgUploaderManager().uploader(capture);
        widget->show();
        widget->activateWindow();
        // NOTE: lambda can't capture 'this' because it might be destroyed later
        CR::ExportTask tasks = tasks;
        QObject::connect(
          widget, &ImgUploaderBase::uploadOk, [=](const QUrl& url) {
              if (ConfigHandler().copyAndCloseAfterUpload()) {
                  if (!(tasks & CR::COPY)) {
                      FlameshotDaemon::copyToClipboard(
                        url.toString(), tr("URL copied to clipboard."));
                      widget->close();
                  } else {
                      widget->showPostUploadDialog();
                  }
              } else {
                  widget->showPostUploadDialog();
              }
          });
    }

    if (!(tasks & CR::UPLOAD)) {
        emit captureTaken(capture);
    }
}

// STATIC ATTRIBUTES
Flameshot::Origin Flameshot::m_origin = Flameshot::DAEMON;
