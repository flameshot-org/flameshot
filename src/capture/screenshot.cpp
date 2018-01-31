// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

#include "src/capture/screenshot.h"
#include "src/capture/widgets/capturebutton.h"
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

QPixmap Screenshot::croppedScreenshot(const QRect &selection) const {
    return m_modifiedScreenshot.copy(selection);
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
    int thickness = modification->thickness();
    modification->tool()->processImage(painter, points, color, thickness);
}


