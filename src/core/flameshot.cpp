// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "flameshot.h"
#include "core/flameshotdaemon.h"
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#include "qhotkey.h"
#endif

#if defined(Q_OS_MACOS)
#include <QWindow>
#include <objc/message.h>

namespace {

constexpr long NSApplicationActivationPolicyRegular = 0;
constexpr long NSApplicationActivationPolicyAccessory = 1;

void setActivationPolicy(long policy)
{
    auto sharedApp = reinterpret_cast<id (*)(id, SEL)>(objc_msgSend);
    auto setPolicy = reinterpret_cast<void (*)(id, SEL, long)>(objc_msgSend);
    id nsApp = sharedApp(reinterpret_cast<id>(objc_getClass("NSApplication")),
                         sel_registerName("sharedApplication"));
    setPolicy(nsApp, sel_registerName("setActivationPolicy:"), policy);
}

void setActivationPolicyRegular()
{
    setActivationPolicy(NSApplicationActivationPolicyRegular);
}

void setActivationPolicyAccessory()
{
    setActivationPolicy(NSApplicationActivationPolicyAccessory);
}

constexpr const char* visibleInDockProperty = "_visibleInDock";

// NSMainMenuWindowLevel (24) + 2: above the menu bar, its status items and
// the Dock, but below pop-up menus so Qt menus remain visible.
constexpr long captureWindowLevel = 26;

// Qt::WindowStaysOnTopHint maps to NSFloatingWindowLevel, which is below the
// menu bar and the Dock, so they would cover parts of the capture overlay.
void raiseAboveMenuBar(QWidget* widget)
{
    auto getWindow = reinterpret_cast<id (*)(id, SEL)>(objc_msgSend);
    auto setLevel = reinterpret_cast<void (*)(id, SEL, long)>(objc_msgSend);
    auto nsView = reinterpret_cast<id>(widget->winId());
    id nsWindow = getWindow(nsView, sel_registerName("window"));
    if (nsWindow != nullptr) {
        setLevel(nsWindow, sel_registerName("setLevel:"), captureWindowLevel);
    }
}

} // namespace

#include <CoreGraphics/CoreGraphics.h>
#endif

#include "config/cacheutils.h"
#include "config/configresolver.h"
#include "config/configwindow.h"
#include "core/qguiappcurrentscreen.h"
#include "utils/abstractlogger.h"
#include "utils/confighandler.h"
#include "utils/desktopinfo.h"
#include "utils/filenamehandler.h"
#include "utils/gifrecorder.h"
#include "utils/screengrabber.h"
#include "utils/screenshotsaver.h"
#include "widgets/capture/capturewidget.h"
#include "widgets/capturelauncher.h"
#include "widgets/gifrecordingoverlay.h"
#include "widgets/gifrecordingwidget.h"
#include "widgets/infowindow.h"

#ifdef ENABLE_IMGUR
#include "tools/imgupload/imguploadermanager.h"
#include "tools/imgupload/storages/imguploaderbase.h"
#include "widgets/imguploaddialog.h"
#include "widgets/uploadhistory.h"
#endif

#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMessageBox>
#include <QScreen>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVersionNumber>

#if defined(Q_OS_MACOS)
#include <QScreen>
#endif

Flameshot::Flameshot()
  : m_haveExternalWidget(false)
  , m_captureWindow(nullptr)
#if (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
  , m_HotkeyScreenshotCapture(nullptr)
#endif
#if (defined(Q_OS_MACOS) && ENABLE_IMGUR)
  , m_HotkeyScreenshotHistory(nullptr)
#endif
{
    QString StyleSheet = CaptureButton::globalStyleSheet();
    qApp->setStyleSheet(StyleSheet);

#if defined(Q_OS_MACOS)
    // Request Screen Recording permission via the proper CoreGraphics API
    if (!CGPreflightScreenCaptureAccess()) {
        CGRequestScreenCaptureAccess();
    }
#endif
#if (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    // Set global shortcuts for MacOS or Windows
    m_HotkeyScreenshotCapture = new QHotkey(
      QKeySequence(ConfigHandler().shortcut("TAKE_SCREENSHOT")), true, this);
    QObject::connect(m_HotkeyScreenshotCapture,
                     &QHotkey::activated,
                     qApp,
                     [this]() { gui(); });
#endif
#if (defined(Q_OS_MACOS) && ENABLE_IMGUR)
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

    CaptureRequest request = req;
    if (request.captureMode() == CaptureRequest::GRAPHICAL_MODE &&
        request.initialSelection().isNull() &&
        ConfigHandler().saveLastRegion()) {
        request.setInitialSelection(getLastRegion());
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

        m_captureWindow = new CaptureWidget(request);

#ifdef Q_OS_WIN
        m_captureWindow->show();
#elif defined(Q_OS_MACOS)
        if (ConfigHandler().useNativeFullscreen()) {
            m_captureWindow->showFullScreen();
        } else {
            m_captureWindow->show();
            raiseAboveMenuBar(m_captureWindow);
        }
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
    if (!resolveAnyConfigErrors()) {
        return;
    }

    bool ok = false;
    QPixmap p;
    QRect geometry;

    if (screenNumber < 0) {
        ScreenGrabber grabber;
        p = grabber.grabEntireDesktop(ok);
        if (ok) {
            QScreen* selectedScreen = grabber.getSelectedScreen();
            if (selectedScreen) {
                geometry = ScreenGrabber().screenGeometry(selectedScreen);
            } else {
                ok = false;
            }
        }
    } else if (screenNumber >= qApp->screens().count()) {
        AbstractLogger() << QObject::tr(
          "Requested screen exceeds screen count");
        ok = false;
    } else {
        // Specific screen number provided - use grabScreen to bypass selector
        QScreen* screen = qApp->screens()[screenNumber];
        p = ScreenGrabber().grabScreen(screen, ok);
        if (ok) {
            geometry = ScreenGrabber().screenGeometry(screen);
        }
    }

    if (ok) {
        QRect region = req.initialSelection();
        if (region.isNull()) {
            region = geometry;
        } else {
            QRect screenGeom = geometry;
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
    if (!resolveAnyConfigErrors()) {
        return;
    }

    bool ok = true;
    QPixmap p(ScreenGrabber().grabFullDesktop(ok));
    if (ok) {
        QRect selection; // `flameshot full` does not support region selection
        exportCapture(p, selection, req);
    } else {
        emit captureFailed();
    }
}

void Flameshot::launcher()
{
    if (!resolveAnyConfigErrors()) {
        return;
    }

    if (m_launcherWindow == nullptr) {
        m_launcherWindow = new CaptureLauncher();
    }
    m_launcherWindow->show();
#if defined(Q_OS_MACOS)
    showDockIcon(m_launcherWindow);
#endif
}

void Flameshot::config()
{
    if (!resolveAnyConfigErrors()) {
        return;
    }

    if (m_configWindow == nullptr) {
        m_configWindow = new ConfigWindow();
        m_configWindow->show();
        // Call show() first, otherwise the correct geometry cannot be fetched
        // for centering the window on the screen
        QRect position = m_configWindow->frameGeometry();
        QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
        position.moveCenter(currentScreen->availableGeometry().center());
        m_configWindow->move(position.topLeft());
#if defined(Q_OS_MACOS)
        showDockIcon(m_configWindow);
#endif
    }
}

void Flameshot::info()
{
    if (m_infoWindow == nullptr) {
        m_infoWindow = new InfoWindow();
#if defined(Q_OS_MACOS)
        showDockIcon(m_infoWindow);
#endif
    }
}

#ifdef ENABLE_IMGUR
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
    // Call show() first, otherwise the correct geometry cannot be fetched
    // for centering the window on the screen
    QRect position = historyWidget->frameGeometry();
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    position.moveCenter(currentScreen->availableGeometry().center());
    historyWidget->move(position.topLeft());

#if defined(Q_OS_MACOS)
    showDockIcon(historyWidget);
#endif
}
#endif

#if defined(Q_OS_MACOS)
void Flameshot::onWindowVisibilityChanged(QWindow::Visibility newVisibility)
{
    auto* qw = qobject_cast<QWindow*>(sender());
    if (!qw) {
        return;
    }

    if (newVisibility == QWindow::Hidden) {
        qw->setProperty(visibleInDockProperty, false);
        --m_dockIconVisibleCount;
        if (m_dockIconVisibleCount == 0) {
            setActivationPolicyAccessory();
        }
    } else {
        bool windowTrackedInDock = qw->property(visibleInDockProperty).toBool();
        if (!windowTrackedInDock) {
            qw->setProperty(visibleInDockProperty, true);
            ++m_dockIconVisibleCount;
            setActivationPolicyRegular();
        }
    }
}

void Flameshot::showDockIcon(QWidget* w)
{
    QWindow* qw = w->windowHandle();
    if (!qw) {
        return;
    }

    connect(qw,
            &QWindow::visibilityChanged,
            this,
            &Flameshot::onWindowVisibilityChanged);
}
#endif

void Flameshot::openSavePath()
{
    QString savePath = ConfigHandler().savePath();
    if (!savePath.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(savePath));
    }
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
    ConfigHandler confighandler;
    if (!confighandler.checkUnrecognizedSettings() ||
        !confighandler.checkSemantics()) {
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
        case CaptureRequest::GIF_MODE: {
            QTimer::singleShot(request.delay(), this, [this, request]() {
                recordGif(request.gifRegion(),
                          request.path(),
                          ConfigHandler().gifFps(),
                          ConfigHandler().gifDuration(),
                          ConfigHandler().gifLoop(),
                          ConfigHandler().gifMaxWidth());
            });
            break;
        }
        default:
            emit captureFailed();
            break;
    }
}

void Flameshot::recordGif(const QRect& region,
                          const QString& path,
                          int fps,
                          int durationSeconds,
                          int loopCount,
                          int maxWidth,
                          bool interactive)
{
    if (m_gifRecorder != nullptr) {
        AbstractLogger::error()
          << tr("A GIF recording is already in progress.");
        emit captureFailed();
        return;
    }
    if (region.isEmpty()) {
        AbstractLogger::error() << tr("There is no region to record.");
        emit captureFailed();
        return;
    }

    fps = qBound(1, fps, 50);
    durationSeconds = qMax(0, durationSeconds);
    loopCount = qMax(0, loopCount);
    maxWidth = qMax(0, maxWidth);
    m_hasGifTextPos = false;

    auto* recorder =
      new GifRecorder(region, fps, durationSeconds, loopCount, maxWidth, this);
    auto* widget = new GifRecordingWidget(region, durationSeconds);
    widget->setFps(fps);
    m_gifRecorder = recorder;
    m_gifWidget = widget;

    GifRecordingOverlay* overlay = nullptr;
    const bool useOverlay = interactive && !DesktopInfo().waylandDetected();
    if (useOverlay) {
        overlay = new GifRecordingOverlay(region);
        m_gifOverlay = overlay;
        widget->setInteractive(true);
        setupGifOverlay(overlay, widget, region);

        overlay->setDrawingColor(ConfigHandler().drawColor());
        overlay->setDrawingWidth(ConfigHandler().drawThickness());

        connect(widget,
                &GifRecordingWidget::toolSelected,
                this,
                [this](GifDrawingTool) { m_hasGifTextPos = false; });
        connect(widget,
                &GifRecordingWidget::textSubmitted,
                this,
                [this, widget]() {
                    if (m_gifOverlay == nullptr) {
                        return;
                    }
                    const QString text = widget->textInput();
                    if (text.isEmpty()) {
                        return;
                    }
                    const QPoint pos =
                      m_hasGifTextPos
                        ? m_gifTextPos
                        : QPoint(m_gifOverlay->width() / 2,
                                 m_gifOverlay->height() / 2);
                    m_gifOverlay->addText(pos, text);
                    m_gifOverlay->finishTool();
                    m_gifOverlay->requestRecreate();
                    m_hasGifTextPos = false;
                });

        recorder->setOverlayProvider(
          [this]() {
              return m_gifOverlay ? m_gifOverlay->renderToImage() : QImage();
          },
          [this]() {
              return m_gifOverlay != nullptr &&
                     m_gifOverlay->canBeHiddenForCapture();
          },
          [this](bool hidden) {
              if (m_gifOverlay != nullptr) {
                  m_gifOverlay->setCaptureHidden(hidden);
              }
          });
    }

    auto cleanup = [this, recorder]() {
        m_hasGifTextPos = false;
        if (m_gifOverlay != nullptr) {
            m_gifOverlay->hide();
            m_gifOverlay->deleteLater();
            m_gifOverlay = nullptr;
        }
        if (m_gifWidget != nullptr) {
            m_gifWidget->hide();
            m_gifWidget->deleteLater();
            m_gifWidget = nullptr;
        }
        if (m_gifRecorder == recorder) {
            m_gifRecorder = nullptr;
        }
        recorder->deleteLater();
    };

    connect(widget,
            &GifRecordingWidget::stopRequested,
            recorder,
            &GifRecorder::stop);
    connect(widget,
            &GifRecordingWidget::cancelRequested,
            recorder,
            &GifRecorder::cancel);
    connect(widget,
            &GifRecordingWidget::fpsChanged,
            recorder,
            &GifRecorder::setFps);
    connect(recorder,
            &GifRecorder::progress,
            widget,
            &GifRecordingWidget::setElapsed);

    connect(recorder,
            &GifRecorder::finished,
            this,
            [this, path, cleanup](const QString& tempFilePath,
                                  int frameCount) {
                if (frameCount <= 0) {
                    AbstractLogger::error() << tr("No frames were recorded.");
                    QFile::remove(tempFilePath);
                    cleanup();
                    emit captureFailed();
                    return;
                }

                if (m_gifWidget != nullptr) {
                    m_gifWidget->hide();
                }
                if (m_gifOverlay != nullptr) {
                    m_gifOverlay->hide();
                }

                QString destination = path;
                bool interactive = destination.isEmpty();
                if (interactive && ConfigHandler().savePathFixed()) {
                    destination = ConfigHandler().savePath();
                    interactive = destination.isEmpty();
                }
                if (interactive) {
                    QString directory = ConfigHandler().savePath();
                    if (directory.isEmpty() || !QDir(directory).exists() ||
                        !QFileInfo(directory).isWritable()) {
                        directory = QStandardPaths::writableLocation(
                          QStandardPaths::PicturesLocation);
                    }
                    const QString suggestion =
                      FileNameHandler().properScreenshotPath(
                        directory, QStringLiteral("gif"));
                    destination = ShowGifSaveFileDialog(suggestion);
                    if (destination.isEmpty()) {
                        QFile::remove(tempFilePath);
                        AbstractLogger::info()
                          << tr("GIF recording discarded.");
                        cleanup();
                        emit captureFailed();
                        return;
                    }
                }

                const bool saved =
                  saveGifToFilesystem(tempFilePath, destination);
                cleanup();
                if (saved) {
                    emit captureTaken(QPixmap());
                } else {
                    emit captureFailed();
                }
            });

    connect(recorder,
            &GifRecorder::failed,
            this,
            [this, cleanup](const QString& error) {
                AbstractLogger::error() << error;
                cleanup();
                emit captureFailed();
            });

    connect(recorder,
            &GifRecorder::cancelled,
            this,
            [this, cleanup]() {
                cleanup();
                emit captureFailed();
            });

    if (overlay != nullptr) {
        overlay->show();
    }
    widget->show();
    recorder->start();
}

void Flameshot::setupGifOverlay(GifRecordingOverlay* overlay,
                                GifRecordingWidget* widget,
                                const QRect& region)
{
    connect(widget,
            &GifRecordingWidget::toolSelected,
            overlay,
            &GifRecordingOverlay::setTool);
    connect(widget,
            &GifRecordingWidget::undoRequested,
            overlay,
            &GifRecordingOverlay::undo);
    connect(widget,
            &GifRecordingWidget::clearRequested,
            overlay,
            &GifRecordingOverlay::clear);
    connect(widget,
            &GifRecordingWidget::colorChanged,
            overlay,
            &GifRecordingOverlay::setDrawingColor);
    connect(widget,
            &GifRecordingWidget::widthChanged,
            overlay,
            &GifRecordingOverlay::setDrawingWidth);
    connect(overlay,
            &GifRecordingOverlay::toolFinished,
            widget,
            &GifRecordingWidget::clearToolSelection);
    connect(overlay,
            &GifRecordingOverlay::textInputRequested,
            this,
            [this, widget, overlay](const QPoint& pos) {
                m_gifTextPos = pos;
                m_hasGifTextPos = true;

                const QString text = widget->textInput();
                if (text.isEmpty()) {
                    // The user clicked the screen before typing anything.
                    // Keep the text tool active and put the keyboard focus
                    // back into the text field instead of cancelling the tool.
                    widget->focusTextInput();
                    return;
                }

                overlay->addText(pos, text);
                overlay->finishTool();
                overlay->requestRecreate();
                m_hasGifTextPos = false;
            });
    connect(overlay,
            &GifRecordingOverlay::recreateRequested,
            this,
            [this, widget, region]() {
                QTimer::singleShot(0, this, [this, widget, region]() {
                    recreateGifOverlay(widget, region);
                });
            });
}

void Flameshot::recreateGifOverlay(GifRecordingWidget* widget,
                                   const QRect& region)
{
    if (m_gifOverlay == nullptr) {
        return;
    }

    auto* old = m_gifOverlay;
    const QVector<GifRecordingOverlay::Stroke> strokes = old->strokes();
    const QColor color = old->drawingColor();
    const int width = old->drawingWidth();

    disconnect(widget, nullptr, old, nullptr);
    delete old;

    auto* fresh = new GifRecordingOverlay(region);
    fresh->setStrokes(strokes);
    fresh->setDrawingColor(color);
    fresh->setDrawingWidth(width);
    m_gifOverlay = fresh;
    setupGifOverlay(fresh, widget, region);
    fresh->show();
    widget->raise();
}

void Flameshot::exportCapture(const QPixmap& capture,
                              QRect& selection,
                              const CaptureRequest& req)
{
    using CR = CaptureRequest;
    int tasks = req.tasks(), mode = req.captureMode();
    QString path = req.path();

    if (tasks & CR::GIF) {
        const QRect region =
          req.hasGifRegion() ? req.gifRegion() : selection;
        recordGif(region,
                  path,
                  ConfigHandler().gifFps(),
                  ConfigHandler().gifDuration(),
                  ConfigHandler().gifLoop(),
                  ConfigHandler().gifMaxWidth(),
                  true);
        return;
    }

    if (tasks & CR::PRINT_GEOMETRY) {
        QTextStream(stdout)
          << selection.width() << "x" << selection.height() << "+"
          << selection.x() << "+" << selection.y() << "\n";
    }

    if (tasks & CR::PRINT_RAW) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        capture.save(&buffer, "PNG");
        if (QFile file; file.open(stdout, QIODevice::WriteOnly)) {
            file.write(byteArray);
            file.close();
        }
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

#ifdef ENABLE_IMGUR
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
          widget, &ImgUploaderBase::uploadOk, [=, this](const QUrl& url) {
              if (ConfigHandler().copyURLAfterUpload()) {
                  if (!(tasks & CR::COPY)) {
                      FlameshotDaemon::copyToClipboard(
                        url.toString(), tr("URL copied to clipboard."));
                  }
                  widget->showPostUploadDialog();
              }
          });
    }
#endif

    if (!(tasks & CR::UPLOAD)) {
        emit captureTaken(capture);
    }
}

void Flameshot::setExternalWidget(bool b)
{
    m_haveExternalWidget = b;
}
bool Flameshot::haveExternalWidget()
{
    return m_haveExternalWidget;
}

// STATIC ATTRIBUTES
Flameshot::Origin Flameshot::m_origin = Flameshot::DAEMON;
