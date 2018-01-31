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

#pragma once

#include <QObject>
#include <QVector>

class QPainter;

class CaptureTool : public QObject
{
    Q_OBJECT

public:
    enum ToolWorkType {
        TYPE_WORKER,
        TYPE_PATH_DRAWER,
        TYPE_LINE_DRAWER
    };

    enum Request {
        REQ_CLOSE_GUI,
        REQ_HIDE_GUI,
        REQ_HIDE_SELECTION,
        REQ_UNDO_MODIFICATION,
        REQ_CLEAR_MODIFICATIONS,
        REQ_SAVE_SCREENSHOT,
        REQ_SELECT_ALL,
        REQ_TO_CLIPBOARD,
        REQ_UPLOAD_TO_IMGUR,
        REQ_MOVE_MODE,
        REQ_OPEN_APP,
    };

    explicit CaptureTool(QObject *parent = nullptr);

    virtual int id() const = 0;
    virtual bool isSelectable() const = 0;
    virtual ToolWorkType toolType() const = 0;

    virtual QString iconName() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;

    virtual void processImage(
            QPainter &painter,
            const QVector<QPoint> &points,
            const QColor &color,
            const int thickness) = 0;

signals:
    void requestAction(Request r);

public slots:
    virtual void onPressed() = 0;

};
