#include "infowindow.h"

#include <QIcon>

InfoWindow::InfoWindow(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(":img/flameshot.svg"));
    setWindowTitle(tr("About"));
    show();
}
