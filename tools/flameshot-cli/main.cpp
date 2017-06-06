#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
//#include <QCommandLineParser>

int main(int argc, char *argv[]) {
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    //QCoreApplication a(argc, argv);
    //QCommandLineParser parser;
    QDBusMessage m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                                    "/",
                                                    "",
                                                    "createCapture"
                                                    );

    QDBusConnection::sessionBus().call(m);


    //return a.exec();

}
