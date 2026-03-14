// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "qrscantool.h"
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QImage>
#include <QLabel>
#include <QTimer>
#include <ZXing/ReadBarcode.h>

QrScanTool::QrScanTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool QrScanTool::closeOnButtonPressed() const
{
    return false;
}

QIcon QrScanTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "qr-code.svg");
}

QString QrScanTool::name() const
{
    return tr("QR Scan");
}

CaptureTool::Type QrScanTool::type() const
{
    return CaptureTool::TYPE_QR_SCAN;
}

QString QrScanTool::description() const
{
    return tr("Scan QR code and copy text to clipboard");
}

CaptureTool* QrScanTool::copy(QObject* parent)
{
    return new QrScanTool(parent);
}

void QrScanTool::pressed(CaptureContext& context)
{
    QPixmap selectedArea = context.selectedScreenshotArea();
    QImage image = selectedArea.toImage().convertToFormat(QImage::Format_RGB888);

    ZXing::ReaderOptions options;
    options.setFormats(ZXing::BarcodeFormat::QRCode);

    ZXing::ImageView imageView(
        image.constBits(),
        image.width(),
        image.height(),
        ZXing::ImageFormat::RGB,
        static_cast<int>(image.bytesPerLine()));

    ZXing::Barcode result = ZXing::ReadBarcode(imageView, options);

    QString message;
    if (result.isValid()) {
        QString text = QString::fromStdString(result.text());
        QApplication::clipboard()->setText(text);
        message = tr("QR code copied to clipboard");
    } else {
        message = tr("No QR code found");
    }

    auto* label = new QLabel(message);
    label->setWindowFlags(Qt::ToolTip);
    label->setStyleSheet(
        "QLabel {"
        "  background-color: #333333;"
        "  color: white;"
        "  padding: 8px 12px;"
        "  border-radius: 4px;"
        "  font-size: 14px;"
        "}");
    label->adjustSize();
    QPoint pos = QCursor::pos();
    label->move(pos.x() + 10, pos.y() + 10);
    label->show();
    QTimer::singleShot(3000, label, &QLabel::deleteLater);
}
