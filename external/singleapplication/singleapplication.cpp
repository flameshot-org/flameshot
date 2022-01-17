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

#include <QtCore/QElapsedTimer>
#include <QtCore/QByteArray>
#include <QtCore/QSharedMemory>

#include "singleapplication.h"
#include "singleapplication_p.h"

/**
 * @brief Constructor. Checks and fires up LocalServer or closes the program
 * if another instance already exists
 * @param argc
 * @param argv
 * @param allowSecondary Whether to enable secondary instance support
 * @param options Optional flags to toggle specific behaviour
 * @param timeout Maximum time blocking functions are allowed during app load
 */
SingleApplication::SingleApplication( int &argc, char *argv[], bool allowSecondary, Options options, int timeout, const QString &userData )
    : app_t( argc, argv ), d_ptr( new SingleApplicationPrivate( this ) )
{
    Q_D( SingleApplication );

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    // On Android and iOS since the library is not supported fallback to
    // standard QApplication behaviour by simply returning at this point.
    qWarning() << "SingleApplication is not supported on Android and iOS systems.";
    return;
#endif

    // Store the current mode of the program
    d->options = options;

    // Add any unique user data
    if ( ! userData.isEmpty() )
        d->addAppData( userData );

    // Generating an application ID used for identifying the shared memory
    // block and QLocalServer
    d->genBlockServerName();

    // To mitigate QSharedMemory issues with large amount of processes
    // attempting to attach at the same time
    SingleApplicationPrivate::randomSleep();

#ifdef Q_OS_UNIX
    // By explicitly attaching it and then deleting it we make sure that the
    // memory is deleted even after the process has crashed on Unix.
    d->memory = new QSharedMemory( d->blockServerName );
    d->memory->attach();
    delete d->memory;
#endif
    // Guarantee thread safe behaviour with a shared memory block.
    d->memory = new QSharedMemory( d->blockServerName );

    // Create a shared memory block
    if( d->memory->create( sizeof( InstancesInfo ) )){
        // Initialize the shared memory block
        if( ! d->memory->lock() ){
          qCritical() << "SingleApplication: Unable to lock memory block after create.";
          abortSafely();
        }
        d->initializeMemoryBlock();
    } else {
        if( d->memory->error() == QSharedMemory::AlreadyExists ){
          // Attempt to attach to the memory segment
          if( ! d->memory->attach() ){
              qCritical() << "SingleApplication: Unable to attach to shared memory block.";
              abortSafely();
          }
          if( ! d->memory->lock() ){
            qCritical() << "SingleApplication: Unable to lock memory block after attach.";
            abortSafely();
          }
        } else {
          qCritical() << "SingleApplication: Unable to create block.";
          abortSafely();
        }
    }

    auto *inst = static_cast<InstancesInfo*>( d->memory->data() );
    QElapsedTimer time;
    time.start();

    // Make sure the shared memory block is initialised and in consistent state
    while( true ){
      // If the shared memory block's checksum is valid continue
      if( d->blockChecksum() == inst->checksum ) break;

      // If more than 5s have elapsed, assume the primary instance crashed and
      // assume it's position
      if( time.elapsed() > 5000 ){
          qWarning() << "SingleApplication: Shared memory block has been in an inconsistent state from more than 5s. Assuming primary instance failure.";
          d->initializeMemoryBlock();
      }

      // Otherwise wait for a random period and try again. The random sleep here
      // limits the probability of a collision between two racing apps and
      // allows the app to initialise faster
      if( ! d->memory->unlock() ){
        qDebug() << "SingleApplication: Unable to unlock memory for random wait.";
        qDebug() << d->memory->errorString();
      }
      SingleApplicationPrivate::randomSleep();
      if( ! d->memory->lock() ){
        qCritical() << "SingleApplication: Unable to lock memory after random wait.";
        abortSafely();
      }
    }

    if( inst->primary == false ){
        d->startPrimary();
        if( ! d->memory->unlock() ){
          qDebug() << "SingleApplication: Unable to unlock memory after primary start.";
          qDebug() << d->memory->errorString();
        }
        return;
    }

    // Check if another instance can be started
    if( allowSecondary ){
        d->startSecondary();
        if( d->options & Mode::SecondaryNotification ){
            d->connectToPrimary( timeout, SingleApplicationPrivate::SecondaryInstance );
        }
        if( ! d->memory->unlock() ){
          qDebug() << "SingleApplication: Unable to unlock memory after secondary start.";
          qDebug() << d->memory->errorString();
        }
        return;
    }

    if( ! d->memory->unlock() ){
      qDebug() << "SingleApplication: Unable to unlock memory at end of execution.";
      qDebug() << d->memory->errorString();
    }

    d->connectToPrimary( timeout, SingleApplicationPrivate::NewInstance );

    delete d;

    ::exit( EXIT_SUCCESS );
}

SingleApplication::~SingleApplication()
{
    Q_D( SingleApplication );
    delete d;
}

/**
 * Checks if the current application instance is primary.
 * @return Returns true if the instance is primary, false otherwise.
 */
bool SingleApplication::isPrimary() const
{
    Q_D( const SingleApplication );
    return d->server != nullptr;
}

/**
 * Checks if the current application instance is secondary.
 * @return Returns true if the instance is secondary, false otherwise.
 */
bool SingleApplication::isSecondary() const
{
    Q_D( const SingleApplication );
    return d->server == nullptr;
}

/**
 * Allows you to identify an instance by returning unique consecutive instance
 * ids. It is reset when the first (primary) instance of your app starts and
 * only incremented afterwards.
 * @return Returns a unique instance id.
 */
quint32 SingleApplication::instanceId() const
{
    Q_D( const SingleApplication );
    return d->instanceNumber;
}

/**
 * Returns the OS PID (Process Identifier) of the process running the primary
 * instance. Especially useful when SingleApplication is coupled with OS.
 * specific APIs.
 * @return Returns the primary instance PID.
 */
qint64 SingleApplication::primaryPid() const
{
    Q_D( const SingleApplication );
    return d->primaryPid();
}

/**
 * Returns the username the primary instance is running as.
 * @return Returns the username the primary instance is running as.
 */
QString SingleApplication::primaryUser() const
{
    Q_D( const SingleApplication );
    return d->primaryUser();
}

/**
 * Returns the username the current instance is running as.
 * @return Returns the username the current instance is running as.
 */
QString SingleApplication::currentUser() const
{
    return SingleApplicationPrivate::getUsername();
}

/**
 * Sends message to the Primary Instance.
 * @param message The message to send.
 * @param timeout the maximum timeout in milliseconds for blocking functions.
 * @return true if the message was sent successfuly, false otherwise.
 */
bool SingleApplication::sendMessage( const QByteArray &message, int timeout )
{
    Q_D( SingleApplication );

    // Nobody to connect to
    if( isPrimary() ) return false;

    // Make sure the socket is connected
    if( ! d->connectToPrimary( timeout,  SingleApplicationPrivate::Reconnect ) )
      return false;

    return d->writeConfirmedMessage( timeout, message );
}

/**
 * Cleans up the shared memory block and exits with a failure.
 * This function halts program execution.
 */
void SingleApplication::abortSafely()
{
    Q_D( SingleApplication );

    qCritical() << "SingleApplication: " << d->memory->error() << d->memory->errorString();
    delete d;
    ::exit( EXIT_FAILURE );
}

QStringList SingleApplication::userData() const
{
    Q_D( const SingleApplication );
    return d->appData();
}
