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
#include "src/capture/tools/capturetool.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/confighandler.h"
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
QPixmap Screenshot::baseScreenshot() const {
    return m_baseScreenshot;
}

//  getScreenshot returns the screenshot with all the modifications
QPixmap Screenshot::screenshot() const {
    return m_modifiedScreenshot;
}

// graphicalSave generates a graphical window to ask about the save path and
// saves the screenshot with all the modifications in such directory
QString Screenshot::graphicalSave(bool &ok,
                                  const QRect &selection,
                                  QWidget *parent) const
{
    ok = false; // user quits the dialog case
    QString savePath = FileNameHandler().absoluteSavePath();
    // setup window
    QFileDialog fileDialog(parent, QObject::tr("Save As"), savePath);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setDirectory(savePath);
    QStringList mimeTypes;
    for (const QByteArray &bf: QImageWriter::supportedMimeTypes())
        mimeTypes.append(QLatin1String(bf));
    fileDialog.setMimeTypeFilters(mimeTypes);
    fileDialog.selectMimeTypeFilter("image/png");
    fileDialog.setDefaultSuffix("png");
    fileDialog.setWindowIcon(QIcon(":img/flameshot.png"));

    QString fileName;
    do {
        if (fileDialog.exec() != QDialog::Accepted) { return QString(); }
        fileName = fileDialog.selectedFiles().first();

        QString pathNoFile = fileName.left(fileName.lastIndexOf("/"));
        ConfigHandler().setSavePath(pathNoFile);

        QPixmap pixToSave;
        if (selection.isEmpty()) {
            pixToSave = m_modifiedScreenshot;
        } else { // save full screen when no selection
            pixToSave = m_modifiedScreenshot.copy(selection);
        }
        ok = pixToSave.save(fileName);
        if (!ok) {
            QMessageBox saveErrBox(
                        QMessageBox::Warning,
                        QObject::tr("Save Error"),
                        QObject::tr("The image could not be saved to \"%1\".")
                        .arg(QDir::toNativeSeparators(fileName)));
            saveErrBox.setWindowIcon(QIcon(":img/flameshot.png"));
            saveErrBox.exec();
        }
    } while(!ok);
    return savePath;
}

QString Screenshot::fileSave(bool &ok, const QRect &selection) const {
    QString savePath = FileNameHandler().absoluteSavePath();
    QPixmap pixToSave;
    if (selection.isEmpty()) {
        pixToSave = m_modifiedScreenshot;
    } else { // save full screen when no selection
        pixToSave = m_modifiedScreenshot.copy(selection);
    }
    ok = pixToSave.save(savePath);
    return savePath;
}

// paintModification adds a new modification to the screenshot
QPixmap Screenshot::paintModification(const CaptureModification *modification) {
    QPainter painter(&m_modifiedScreenshot);
    painter.setRenderHint(QPainter::Antialiasing);
    paintInPainter(painter, modification);
    return m_modifiedScreenshot;
}

// paintTemporalModification paints a modification without updating the
// member pixmap
QPixmap Screenshot::paintTemporalModification(
        const CaptureModification *modification)
{
    QPixmap tempPix(m_modifiedScreenshot);
    QPainter painter(&tempPix);
    if (modification->buttonType() != CaptureButton::TYPE_PENCIL) {
        painter.setRenderHint(QPainter::Antialiasing);
    }
    paintInPainter(painter, modification);
    return tempPix;
}

// paintBaseModifications overrides the modifications of the screenshot
// with new ones.
QPixmap Screenshot::overrideModifications(
        const QVector<CaptureModification*> &m)
{
    m_modifiedScreenshot = m_baseScreenshot;
    for (const CaptureModification *const modification: m) {
        paintModification(modification);
    }
    return m_modifiedScreenshot;
}

// paintInPainter is an aux method to prevent duplicated code, it draws the
// passed modification to the painter.
void Screenshot::paintInPainter(QPainter &painter,
                                const CaptureModification *modification)
{
    const QVector<QPoint> &points = modification->points();
    QColor color = modification->color();
    modification->tool()->processImage(painter, points, color);
}

void Screenshot::uploadToImgur(QNetworkAccessManager *accessManager,
                               const QRect &selection)
{
    QString title ="flameshot_screenshot";
    QString description = FileNameHandler().parsedPattern();
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


