// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "qrtool.h"
#include "qrwidget.h"
#include "core/flameshot.h"
#include "core/flameshotdaemon.h"
#include "tools/capturecontext.h"
#include "utils/abstractlogger.h"
#include "utils/colorutils.h"

#include <QApplication>
#include <QClipboard>

// ZBar C interface declarations
extern "C" {
    struct zbar_image_s;
    typedef struct zbar_image_s zbar_image_t;
    struct zbar_image_scanner_s;
    typedef struct zbar_image_scanner_s zbar_image_scanner_t;
    struct zbar_symbol_s;
    typedef struct zbar_symbol_s zbar_symbol_t;
    struct zbar_symbol_set_s;
    typedef struct zbar_symbol_set_s zbar_symbol_set_t;

    typedef enum zbar_symbol_type_e {
        ZBAR_NONE = 0,
        ZBAR_QRCODE = 64
    } zbar_symbol_type_t;

    typedef enum zbar_config_e {
        ZBAR_CFG_ENABLE = 0
    } zbar_config_t;

    zbar_image_scanner_t* zbar_image_scanner_create(void);
    void zbar_image_scanner_destroy(zbar_image_scanner_t* scanner);
    int zbar_image_scanner_set_config(zbar_image_scanner_t* scanner, int sym, int cfg, int val);
    zbar_image_t* zbar_image_create(void);
    void zbar_image_destroy(zbar_image_t* image);
    void zbar_image_set_format(zbar_image_t* image, unsigned long format);
    void zbar_image_set_size(zbar_image_t* image, unsigned width, unsigned height);
    void zbar_image_set_data(zbar_image_t* image, const void* data, unsigned long data_byte_length, void (*cleanup)(zbar_image_t* image));
    int zbar_scan_image(zbar_image_scanner_t* scanner, zbar_image_t* image);
    const zbar_symbol_set_t* zbar_image_get_symbols(const zbar_image_t* image);
    const zbar_symbol_t* zbar_symbol_set_first_symbol(const zbar_symbol_set_t* symbols);
    const zbar_symbol_t* zbar_symbol_next(const zbar_symbol_t* symbol);
    const char* zbar_symbol_get_data(const zbar_symbol_t* symbol);
    int zbar_symbol_get_type(const zbar_symbol_t* symbol);
    const char* zbar_get_symbol_name(int sym);
}

QrTool::QrTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool QrTool::closeOnButtonPressed() const
{
    return true;
}

QIcon QrTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    if (ColorUtils::colorIsDark(background)) {
        return QIcon(QStringLiteral(":/qrplugin/icons/white_qr.svg"));
    } else {
        return QIcon(QStringLiteral(":/qrplugin/icons/black_qr.svg"));
    }
}

QString QrTool::name() const
{
    return tr("QR & Barcode Scanner");
}

CaptureTool::Type QrTool::type() const
{
    return CaptureTool::NONE;
}

QString QrTool::description() const
{
    return tr("Scan QR codes & barcodes, open URLs, and generate custom QR codes");
}

CaptureTool* QrTool::copy(QObject* parent)
{
    return new QrTool(parent);
}

void QrTool::pressed(CaptureContext& context)
{
    QPixmap capture = context.selectedScreenshotArea();
    if (capture.isNull() || capture.width() < 4 || capture.height() < 4) {
        capture = context.screenshot;
    }

    if (capture.isNull()) {
        emit requestAction(REQ_CLOSE_GUI);
        return;
    }

    // Convert to grayscale for ZBar scanning
    QImage grayImg = capture.toImage().convertToFormat(QImage::Format_Grayscale8);
    const int w = grayImg.width();
    const int h = grayImg.height();

    QString scannedData;
    QString symbolTypeName = QStringLiteral("QR Code");

    zbar_image_scanner_t* scanner = zbar_image_scanner_create();
    if (scanner) {
        zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);
        zbar_image_t* image = zbar_image_create();
        if (image) {
            // 'Y800' is standard 8-bit grayscale in fourcc format
            unsigned long fourcc_y800 = (unsigned long)'Y' |
                                        ((unsigned long)'8' << 8) |
                                        ((unsigned long)'0' << 16) |
                                        ((unsigned long)'0' << 24);
            zbar_image_set_format(image, fourcc_y800);
            zbar_image_set_size(image, w, h);
            zbar_image_set_data(image, grayImg.constBits(), (unsigned long)w * h, nullptr);

            int n = zbar_scan_image(scanner, image);
            if (n > 0) {
                const zbar_symbol_set_t* syms = zbar_image_get_symbols(image);
                if (syms) {
                    const zbar_symbol_t* sym = zbar_symbol_set_first_symbol(syms);
                    if (sym) {
                        const char* data = zbar_symbol_get_data(sym);
                        if (data) {
                            scannedData = QString::fromUtf8(data);
                        }
                        int stype = zbar_symbol_get_type(sym);
                        const char* sname = zbar_get_symbol_name(stype);
                        if (sname) {
                            symbolTypeName = QString::fromLatin1(sname);
                        }
                    }
                }
            }
            zbar_image_destroy(image);
        }
        zbar_image_scanner_destroy(scanner);
    }

    if (!scannedData.isEmpty()) {
        QApplication::clipboard()->setText(scannedData);
        AbstractLogger::info()
          << tr("Decoded %1: %2 (copied to clipboard)").arg(symbolTypeName, scannedData);
    } else {
        AbstractLogger::info()
          << tr("No QR code or barcode detected in selection.");
    }

    Flameshot::instance()->setExternalWidget(true);

    auto* widget = new QrWidget(scannedData, symbolTypeName, capture, nullptr);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(widget, &QWidget::destroyed, []() {
        Flameshot::instance()->setExternalWidget(false);
        if (FlameshotDaemon::instance() == nullptr) {
            qApp->quit();
        }
    });
    widget->show();
    widget->raise();
    widget->activateWindow();

    emit requestAction(REQ_CLEAR_SELECTION);
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_CLOSE_GUI);
}
