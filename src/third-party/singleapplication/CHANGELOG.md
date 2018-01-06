Changelog
=========

__3.0.10__
----------

* Removed C style casts and eliminated all clang warnings. Fixed `instanceId`
  reading from only one byte in the message deserialization. Cleaned up
  serialization code using `QDataStream`. Changed connection type to use
  `quint8 enum` rather than `char`.
* Renamed `SingleAppConnectionType` to `ConnectionType`. Added initialization
  values to all `ConnectionType` enum cases.

    _Jedidiah Buck McCready_

__3.0.9__
---------

*   Added SingleApplicationPrivate::primaryPid() as a solution to allow
    bringing the primary window of an application to the foreground on
    Windows.

    _Eelco van Dam from Peacs BV_

__3.0.8__
---------

*   Bug fix - changed QApplication::instance() to QCoreApplication::instance()

    _Evgeniy Bazhenov_

__3.0.7a__
----------

*   Fixed compilation error with Mingw32 in MXE thanks to Vitaly Tonkacheyev.
*   Removed QMutex used for thread safe behaviour. The implementation now uses
    QCoreApplication::instance() to get an instance to SingleApplication for
    memory deallocation.

__3.0.6a__
----------

*   Reverted GetUserName API usage on Windows. Fixed bug with missing library.
*   Fixed bug in the Calculator example, preventing it's window to be raised
    on Windows.

    Special thanks to Charles Gunawan.

__3.0.5a__
----------

*   Fixed a memory leak in the SingleApplicationPrivate destructor.

    _Sergei Moiseev_

__3.0.4a__
----------

*   Fixed shadow and uninitialised variable warnings.

    _Paul Walmsley_

__3.0.3a__
----------

*   Removed Microsoft Windows specific code for getting username due to
    multiple problems and compiler differences on Windows platforms. On
    Windows the shared memory block in User mode now includes the user's
    home path (which contains the user's username).

*   Explicitly getting absolute path of the user's home directory as on Unix
    a relative path (`~`) may be returned.

__3.0.2a__
----------

*   Fixed bug on Windows when username containing wide characters causes the
    library to crash.

    _Le Liu_

__3.0.1a__
----------

*   Allows the application path and version to be excluded from the server name
    hash. The following flags were added for this purpose:
      * `SingleApplication::Mode::ExcludeAppVersion`
      * `SingleApplication::Mode::ExcludeAppPath`
*   Allow a non elevated process to connect to a local server created by an
    elevated process run by the same user on Windows
*   Fixes a problem with upper case letters in paths on Windows

    _Le Liu_

__v3.0a__
---------

*   Depricated secondary instances count.
*   Added a sendMessage() method to send a message to the primary instance.
*   Added a receivedMessage() signal, emitted when a message is received from a
    secondary instance.
*   The SingleApplication constructor's third parameter is now a bool
    specifying if the current instance should be allowed to run as a secondary
    instance if there is already a primary instance.
*   The SingleApplication constructor accept a fourth parameter specifying if
    the SingleApplication block should be User-wide or System-wide.
*   SingleApplication no longer relies on `applicationName` and
    `organizationName` to be set. It instead concatenates all of the following
    data and computes a `SHA256` hash which is used as the key of the
    `QSharedMemory` block and the `QLocalServer`. Since at least
    `applicationFilePath` is always present there is no need to explicitly set
    any of the following prior to initialising `SingleApplication`.
      * `QCoreApplication::applicationName`
      * `QCoreApplication::applicationVersion`
      * `QCoreApplication::applicationFilePath`
      * `QCoreApplication::organizationName`
      * `QCoreApplication::organizationDomain`
      * User name or home directory path if in User mode
*   The primary instance is no longer notified when a secondary instance had
    been started by default. A `Mode` flag for this feature exists.
*   Added `instanceNumber()` which represents a unique identifier for each
    secondary instance started. When called from the primary instance will
    return `0`.

__v2.4__
--------

*   Stability improvements
*   Support for secondary instances.
*   The library now recovers safely after the primary process has crashed
and the shared memory had not been deleted.

__v2.3__
--------

*   Improved pimpl design and inheritance safety.

    _Vladislav Pyatnichenko_

__v2.2__
--------

*   The `QAPPLICATION_CLASS` macro can now be defined in the file including the
Single Application header or with a `DEFINES+=` statement in the project file.

__v2.1__
--------

*   A race condition can no longer occur when starting two processes nearly
    simultaneously.

    Fix issue [#3](https://github.com/itay-grudev/SingleApplication/issues/3)

__v2.0__
--------

*   SingleApplication is now being passed a reference to `argc` instead of a
    copy.

    Fix issue [#1](https://github.com/itay-grudev/SingleApplication/issues/1)

*   Improved documentation.
