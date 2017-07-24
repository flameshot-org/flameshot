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

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QPixmap>
#include <QRect>
#include <QPointer>
#include <QObject>

class QString;
class CaptureModification;
class QNetworkAccessManager;

class Screenshot : public QObject {
   Q_OBJECT
public:
    Screenshot(const QPixmap &, QObject *parent = nullptr);
    ~Screenshot();

    void setScreenshot(const QPixmap &);
    QPixmap getBaseScreenshot() const;
    QPixmap getScreenshot() const;

    QString graphicalSave(bool &ok,
                          const QRect &selection = QRect(),
                          QWidget *parent = 0) const;
    QString fileSave(bool &ok, const QRect &selection = QRect()) const;
    void uploadToImgur(QNetworkAccessManager *,
                       const QRect &selection = QRect());
    QPixmap paintModification(const CaptureModification*);
    QPixmap paintTemporalModification(const CaptureModification*);
    QPixmap paintBaseModifications(const QVector<CaptureModification*> &);

private:
    QPixmap m_baseScreenshot;
    QPixmap m_modifiedScreenshot;
    QPointer<QNetworkAccessManager> m_accessManager;

    void paintInPainter(QPainter &, const CaptureModification *);
};

#endif // SCREENSHOT_H
