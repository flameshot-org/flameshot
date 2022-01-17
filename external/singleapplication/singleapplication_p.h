// The MIT License (MIT)
//
// Copyright (c) Itay Grudev 2015 - 2020
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
    qint64 primaryPid;
    char primaryUser[128];
    quint16 checksum; // Must be the last field
};

struct ConnectionInfo {
    qint64 msgLen = 0;
    quint32 instanceId = 0;
    quint8 stage = 0;
};

class SingleApplicationPrivate : public QObject {
Q_OBJECT
public:
    enum ConnectionType : quint8 {
        InvalidConnection = 0,
        NewInstance = 1,
        SecondaryInstance = 2,
        Reconnect = 3
    };
    enum ConnectionStage : quint8 {
        StageInitHeader = 0,
        StageInitBody = 1,
        StageConnectedHeader = 2,
        StageConnectedBody = 3,
    };
    Q_DECLARE_PUBLIC(SingleApplication)

    SingleApplicationPrivate( SingleApplication *q_ptr );
    ~SingleApplicationPrivate() override;

    static QString getUsername();
    void genBlockServerName();
    void initializeMemoryBlock() const;
    void startPrimary();
    void startSecondary();
    bool connectToPrimary( int msecs, ConnectionType connectionType );
    quint16 blockChecksum() const;
    qint64 primaryPid() const;
    QString primaryUser() const;
    bool isFrameComplete(QLocalSocket *sock);
    void readMessageHeader(QLocalSocket *socket, ConnectionStage nextStage);
    void readInitMessageBody(QLocalSocket *socket);
    void writeAck(QLocalSocket *sock);
    bool writeConfirmedFrame(int msecs, const QByteArray &msg);
    bool writeConfirmedMessage(int msecs, const QByteArray &msg);
    static void randomSleep();
    void addAppData(const QString &data);
    QStringList appData() const;

    SingleApplication *q_ptr;
    QSharedMemory *memory;
    QLocalSocket *socket;
    QLocalServer *server;
    quint32 instanceNumber;
    QString blockServerName;
    SingleApplication::Options options;
    QMap<QLocalSocket*, ConnectionInfo> connectionMap;
    QStringList appDataList;

public Q_SLOTS:
    void slotConnectionEstablished();
    void slotDataAvailable( QLocalSocket*, quint32 );
    void slotClientConnectionClosed( QLocalSocket*, quint32 );
};

#endif // SINGLEAPPLICATION_P_H
