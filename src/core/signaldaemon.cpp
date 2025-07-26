#include "signaldaemon.h"
#include <QApplication>
#include <QSocketNotifier>
#include <csignal>
#include <qdebug.h>
#include <sys/socket.h>
#include <unistd.h>

int SignalDaemon::sigintFd[2];
int SignalDaemon::sigtermFd[2];

SignalDaemon::SignalDaemon(QObject* parent)
  : QObject(parent)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigintFd))
        qFatal("Couldn't create INT socketpair");

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd))
        qFatal("Couldn't create TERM socketpair");
    snInt = new QSocketNotifier(sigintFd[1], QSocketNotifier::Read, this);
    connect(
      snInt, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigInt()));
    snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    connect(snTerm,
            SIGNAL(activated(QSocketDescriptor)),
            this,
            SLOT(handleSigTerm()));
}

void SignalDaemon::intSignalHandler(int)
{
    char a = 1;
    ::write(sigintFd[0], &a, sizeof(a));
}

void SignalDaemon::termSignalHandler(int)
{
    char a = 1;
    ::write(sigtermFd[0], &a, sizeof(a));
}

void SignalDaemon::handleSigTerm()
{
    snTerm->setEnabled(false);
    char tmp;
    ::read(sigtermFd[1], &tmp, sizeof(tmp));

    QApplication::quit();
    snTerm->setEnabled(true);
}

void SignalDaemon::handleSigInt()
{
    snInt->setEnabled(false);

    char tmp;
    ::read(sigintFd[1], &tmp, sizeof(tmp));

    QApplication::quit();

    snInt->setEnabled(true);
}
