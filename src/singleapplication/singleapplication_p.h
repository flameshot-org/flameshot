// The MIT License (MIT)
//
// Copyright (c) Itay Grudev 2015 - 2016
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the SingleApplication API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#ifndef SINGLEAPPLICATION_P_H
#define SINGLEAPPLICATION_P_H

#include <QtCore/QSharedMemory>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include "singleapplication.h"

struct InstancesInfo {
    bool primary;
    quint32 secondary;
};

class SingleApplicationPrivate : public QObject {
Q_OBJECT
public:
    Q_DECLARE_PUBLIC(SingleApplication)

    SingleApplicationPrivate( SingleApplication *q_ptr );
     ~SingleApplicationPrivate();

    void genBlockServerName( int msecs );
    void startPrimary( bool resetMemory );
    void startSecondary();
    void connectToPrimary( int msecs, char connectionType );

#ifdef Q_OS_UNIX
    void crashHandler();
    static void terminate( int signum );
#endif

    QSharedMemory *memory;
    SingleApplication *q_ptr;
    QLocalSocket *socket;
    QLocalServer *server;
    quint32 instanceNumber;
    QString blockServerName;
    SingleApplication::Options options;

public Q_SLOTS:
    void slotConnectionEstablished();
    void slotDataAvailable( QLocalSocket*, quint32 );
    void slotClientConnectionClosed( QLocalSocket*, quint32 );
};

#endif // SINGLEAPPLICATION_P_H
