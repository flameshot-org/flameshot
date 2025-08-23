#include <QObject>
#include <QSocketNotifier>

class SignalDaemon : public QObject
{
    Q_OBJECT

public:
    SignalDaemon(QObject* parent = 0);
    ~SignalDaemon() = default;

    // Unix signal handlers.
    static void intSignalHandler(int unused);
    static void termSignalHandler(int unused);

public slots:
    // Qt signal handlers.
    void handleSigInt();
    void handleSigTerm();

private:
    static int sigintFd[2];
    static int sigtermFd[2];

    QSocketNotifier* snInt;
    QSocketNotifier* snTerm;
};