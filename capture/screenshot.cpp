#include "screenshot.h"
#include "capturemodification.h"
#include "button.h"
#include <QStandardPaths>
#include <QIcon>
#include <QSettings>
#include <QObject>
#include <QString>
#include <QMessageBox>
#include <QImageWriter>
#include <QFileDialog>
#include <QPainter>

// ADD MODIFICATIONS
Screenshot::Screenshot(const QPixmap &p) : m_screenshot(p) {

}

void Screenshot::setScreenshot(const QPixmap &p) {
    m_screenshot = p;
}

QPixmap Screenshot::getScreenshot() const {
    return m_screenshot;
}
/*
https://github.com/KDE/spectacle/tree/118bcd8a9a4c6c89445a589fa990d15ec9223099/src/PlatformBackends
https://github.com/ckaiser/Lightscreen/blob/354574e5d4a15af60004c86cb747dc3bb72a33e8/tools/screenshot.cpp#L416
*/
QString Screenshot::graphicalSave(const QRect &selection) const {
    const QString format = "png";

    QSettings settings;
    QString savePath = settings.value("savePath").toString();
    if (savePath.isEmpty()) {
        savePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        if (savePath.isEmpty())
            savePath = QDir::currentPath();
    }
    // find unused name adding _n where n is a number
    QString tempName = QObject::tr("/screenshot");
    QFileInfo checkFile(savePath + tempName + "." + format);
    if (checkFile.exists()) {
        tempName += "_";
        int i = 1;
        while (true) {
            checkFile.setFile(
                        savePath + tempName + QString::number(i) + "." + format);
            if (!checkFile.exists()) {
                tempName += QString::number(i);
                break;
            }
            ++i;
        }
    }
    savePath += tempName + "." + format;

    QFileDialog fileDialog(nullptr, QObject::tr("Save As"), savePath);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setDirectory(savePath);
    QStringList mimeTypes;
    for (const QByteArray &bf: QImageWriter::supportedMimeTypes())
        mimeTypes.append(QLatin1String(bf));
    fileDialog.setMimeTypeFilters(mimeTypes);
    fileDialog.selectMimeTypeFilter("image/" + format);
    fileDialog.setDefaultSuffix(format);
    fileDialog.setWindowIcon(QIcon(":img/flameshot.svg"));
    if (fileDialog.exec() != QDialog::Accepted) { return ""; }
    const QString fileName = fileDialog.selectedFiles().first();

    const QString pathNoFile = fileName.left(fileName.lastIndexOf("/"));
    settings.setValue("savePath", pathNoFile);

    QPixmap pixToSave;
    if (selection.isEmpty()) {
        pixToSave = m_screenshot;
    } else { // save full screen when no selection
        pixToSave = m_screenshot.copy(selection);
    }

    if (settings.value("mouseVisible").toBool()) {
        // TO DO
    }

    if (!pixToSave.save(fileName)) {
        QMessageBox::warning(nullptr, QObject::tr("Save Error"),
                             QObject::tr("The image could not be saved to \"%1\".")
                             .arg(QDir::toNativeSeparators(fileName)));
    }
    return fileName;
}

QPixmap Screenshot::paintModifications(const QVector<CaptureModification> v) {
    QPainter painter(&m_screenshot);
    painter.setRenderHint(QPainter::Antialiasing);
    for (CaptureModification modification: v) {
        painter.setPen(QPen(modification.getColor(), 2));
        QVector<QPoint> points = modification.getPoints();
        switch (modification.getType()) {
        case Button::Type::arrow:
            painter.drawLine(points[0], points[1]);
            break;
        case Button::Type::circle:
            painter.drawEllipse(QRect(points[0], points[1]));
            break;
        case Button::Type::line:
            painter.drawLine(points[0], points[1]);
            break;
        case Button::Type::marker:
            painter.setOpacity(0.5);
            painter.setPen(QPen(modification.getColor(), 14));
            painter.drawLine(points[0], points[1]);
            painter.setOpacity(1);
            break;
        case Button::Type::pencil:
            painter.drawPolyline(points.data(), points.size());
            break;
        case Button::Type::rectangle:
            painter.drawRect(QRect(points[0], points[1]));
            break;
        default:
            break;
        }
    }
    return m_screenshot;
}


