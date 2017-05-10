#include "controller.h"
#include <QApplication>
#include "singleapplication.h"

#include <QFile>

int main(int argc, char *argv[]) {
    SingleApplication app(argc, argv);
    app.setApplicationName("flameshot");
    app.setOrganizationName("Dharkael");

    QFile file(":/styles/button.css");
    if(file.open(QFile::ReadOnly)) {
       QString StyleSheet = QLatin1String(file.readAll());
       app.setStyleSheet(StyleSheet);
    }

    Controller w;

    return app.exec();
}
