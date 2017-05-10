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

#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>

class NativeEventFilter : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
public:
    explicit NativeEventFilter(QObject *parent = 0);

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

signals:
    void activated();

public slots:

private:
    void setShortcut();
};

#endif // NATIVEEVENTFILTER_H
