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

#ifndef GRAPHICALSCREENSHOTSAVER_H
#define GRAPHICALSCREENSHOTSAVER_H

#include <QWidget>

class QFileDialog;
class QVBoxLayout;

class GraphicalScreenshotSaver : public QWidget
{
    Q_OBJECT
public:
    explicit GraphicalScreenshotSaver(const QPixmap &capture,
                                      QWidget *parent = nullptr);

private:
    QPixmap m_pixmap;
    QFileDialog *m_fileDialog;
    QVBoxLayout *m_layout;

    void initFileDialog();
    void showErrorMessage(const QString &msg);
    void checkSaveAcepted();
};

#endif // GRAPHICALSCREENSHOTSAVER_H
