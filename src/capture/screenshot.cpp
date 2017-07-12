// Copyright 2017 Alejandro Sirgo Rica
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "screenshot.h"
#include "capturebutton.h"
#include "capturemodification.h"
#include "src/utils/filenamehandler.h"
#include <QStandardPaths>
#include <QIcon>
#include <QSettings>
#include <QObject>
#include <QString>
#include <QMessageBox>
#include <QImageWriter>
#include <QFileDialog>
#include <QPainter>
#include <QBuffer>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

// Screenshot is an extension of QPixmap which lets you manage specific tasks

Screenshot::Screenshot(const QPixmap &p, QObject *parent) : QObject(parent),
    m_baseScreenshot(p),
    m_modifiedScreenshot(p)
{

}

Screenshot::~Screenshot() {
}

void Screenshot::setScreenshot(const QPixmap &p) {
    m_baseScreenshot = p;
    m_modifiedScreenshot = p;
}

//  getScreenshot returns the screenshot with no modifications
QPixmap Screenshot::getBaseScreenshot() const {
    return m_baseScreenshot;
}

//  getScreenshot returns the screenshot with all the modifications
QPixmap Screenshot::getScreenshot() const {
    return m_modifiedScreenshot;
}

// graphicalSave generates a graphical window to ask about the save path and
// saves the screenshot with all the modifications in such directory
QString Screenshot::graphicalSave(const QRect &selection, QWidget *parent) const {
    const QString format = "png";
    QSettings settings;
    QString savePath = settings.value("savePath").toString();
    if (savePath.isEmpty() || !QDir(savePath).exists()) {
        savePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        if (savePath.isEmpty()) {
            savePath = QDir::currentPath();
        }
    }

    QString tempName = "/"+ FileNameHandler().getParsedPattern();
    // find unused name adding _n where n is a number
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

    QFileDialog fileDialog(parent, QObject::tr("Save As"), savePath);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setDirectory(savePath);
    QStringList mimeTypes;
    for (const QByteArray &bf: QImageWriter::supportedMimeTypes())
        mimeTypes.append(QLatin1String(bf));
    fileDialog.setMimeTypeFilters(mimeTypes);
    fileDialog.selectMimeTypeFilter("image/" + format);
    fileDialog.setDefaultSuffix(format);
    fileDialog.setWindowIcon(QIcon(":img/flameshot.png"));

    bool saved = false;
    QString fileName, pathNoFile;
    do {
        if (fileDialog.exec() != QDialog::Accepted) { return ""; }
        fileName = fileDialog.selectedFiles().first();

        pathNoFile = fileName.left(fileName.lastIndexOf("/"));
        settings.setValue("savePath", pathNoFile);

        QPixmap pixToSave;
        if (selection.isEmpty()) {
            pixToSave = m_modifiedScreenshot;
        } else { // save full screen when no selection
            pixToSave = m_modifiedScreenshot.copy(selection);
        }

        saved = pixToSave.save(fileName);
        if (!saved) {
            QMessageBox saveErrBox(QMessageBox::Warning, QObject::tr("Save Error"),
                                   QObject::tr("The image could not be saved to \"%1\".")
                                   .arg(QDir::toNativeSeparators(fileName)));
            saveErrBox.setWindowIcon(QIcon(":img/flameshot.png"));
            saveErrBox.exec();
        }
    } while(!saved);

    return pathNoFile;
}

QString Screenshot::fileSave(const QRect &selection) const {
    const QString format = "png";
    QSettings settings;
    QString savePath = settings.value("savePath").toString();
    if (savePath.isEmpty() || !QDir(savePath).exists()) {
        savePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        if (savePath.isEmpty()) {
            savePath = QDir::currentPath();
        }
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
    QPixmap pixToSave;
    if (selection.isEmpty()) {
        pixToSave = m_modifiedScreenshot;
    } else { // save full screen when no selection
        pixToSave = m_modifiedScreenshot.copy(selection);
    }
    return pixToSave.save(savePath) ? savePath : "";
}

// paintModification adds a new modification to the screenshot
QPixmap Screenshot::paintModification(const CaptureModification &modification) {
    QPainter painter(&m_modifiedScreenshot);
    painter.setRenderHint(QPainter::Antialiasing);

    paintInPainter(painter, modification);
    return m_modifiedScreenshot;
}

// paintTemporalModification paints a modification without updating the
// member pixmap
QPixmap Screenshot::paintTemporalModification(
        const CaptureModification &modification)
{
    QPixmap tempPix = m_modifiedScreenshot;
    QPainter painter(&tempPix);
    if (modification.getType() != CaptureButton::Type::pencil) {
        painter.setRenderHint(QPainter::Antialiasing);
    }
    paintInPainter(painter, modification);
    return tempPix;
}

// paintBaseModifications overrides the modifications of the screenshot
// with new ones.
QPixmap Screenshot::paintBaseModifications(
        const QVector<CaptureModification> &m)
{
    m_modifiedScreenshot = m_baseScreenshot;
    for (const CaptureModification modification: m) {
        paintModification(modification);
    }
    return m_modifiedScreenshot;
}

namespace {
    const int ArrowWidth = 10;
    const int ArrowHeight = 18;

    QPainterPath getArrowHead(QPoint p1, QPoint p2) {
        QLineF body(p1, p2);
        int originalLength = body.length();
        body.setLength(ArrowWidth);
        // move across the line up to the head
        QLineF temp(QPoint(0,0), p2-p1);
        temp.setLength(originalLength-ArrowHeight);
        QPointF bottonTranslation(temp.p2());

        // generates the transformation to center of the arrowhead
        body.setAngle(body.angle()+90);
        QPointF temp2 = p1-body.p2();
        QPointF centerTranslation((temp2.x()/2), (temp2.y()/2));

        body.translate(bottonTranslation);
        body.translate(centerTranslation);

        QPainterPath path;
        path.moveTo(p2);
        path.lineTo(body.p1());
        path.lineTo(body.p2());
        path.lineTo(p2);
        return path;
    }

    // gets a shorter line to prevent overlap in the point of the arrow
    QLine getShorterLine(QPoint p1, QPoint p2) {
        QLineF l(p1, p2);
        l.setLength(l.length()-ArrowHeight);
        return l.toLine();
    }

}




// paintInPainter is an aux method to prevent duplicated code, it draws the
// passed modification to the painter.
void Screenshot::paintInPainter(QPainter &painter,
                                const CaptureModification &modification)
{
    painter.setPen(QPen(modification.getColor(), 2));
    QVector<QPoint> points = modification.getPoints();
    switch (modification.getType()) {
    case CaptureButton::Type::arrow:
        painter.drawLine(getShorterLine(points[0], points[1]));
        painter.fillPath(getArrowHead(points[0], points[1]),
                QBrush(modification.getColor()));
        break;
    case CaptureButton::Type::circle:
        painter.drawEllipse(QRect(points[0], points[1]));
        break;
    case CaptureButton::Type::line:
        painter.drawLine(points[0], points[1]);
        break;
    case CaptureButton::Type::marker:
        painter.setOpacity(0.35);
        painter.setPen(QPen(modification.getColor(), 14));
        painter.drawLine(points[0], points[1]);
        painter.setOpacity(1);
        break;
    case CaptureButton::Type::pencil:
        painter.drawPolyline(points.data(), points.size());
        break;
    case CaptureButton::Type::selection:
        painter.drawRect(QRect(points[0], points[1]));
        break;
    case CaptureButton::Type::rectangle:
        painter.setBrush(QBrush(modification.getColor()));
        painter.drawRect(QRect(points[0], points[1]));
        break;
    default:
        break;
    }
}

void Screenshot::uploadToImgur(QNetworkAccessManager *accessManager,
                               const QRect &selection)
{
    QString title ="flameshot_screenshot";
    QString description = FileNameHandler().getParsedPattern();
    QPixmap pixToSave;
    if (selection.isEmpty()) {
        pixToSave = m_modifiedScreenshot;
    } else { // save full screen when no selection
        pixToSave = m_modifiedScreenshot.copy(selection);
    }
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixToSave.save(&buffer, "PNG");

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("title", title);
    urlQuery.addQueryItem("description", description);

    QNetworkRequest request;
    QUrl url("https://api.imgur.com/3/image");
    url.setQuery(urlQuery);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/application/x-www-form-urlencoded");
    request.setRawHeader("Authorization", "Client-ID 313baf0c7b4d3ff");

    accessManager->post(request, byteArray);
}


