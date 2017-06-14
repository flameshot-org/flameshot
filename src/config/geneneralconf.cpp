#include "geneneralconf.h"

#include <QVBoxLayout>
#include <QSettings>
#include <QCheckBox>

GeneneralConf::GeneneralConf(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel);

    m_layout = new QVBoxLayout(this);
    initShowHelp();
    initShowDesktopNotification();
}

void GeneneralConf::showHelpChanged(bool checked) {
    QSettings().setValue("showHelp", checked);
}

void GeneneralConf::showDesktopNotificationChanged(bool checked) {
    QSettings().setValue("showDesktopNotification", checked);
}

void GeneneralConf::initShowHelp() {
    QCheckBox *c = new QCheckBox(tr("Show help message"), this);
    QSettings settings;
    bool checked = settings.value("showHelp").toBool();
    c->setChecked(checked);
    c->setToolTip(tr("Show the help message at the beginning "
                       "in the capture mode."));
    m_layout->addWidget(c);

    connect(c, &QCheckBox::clicked, this, &GeneneralConf::showHelpChanged);
}

void GeneneralConf::initShowDesktopNotification() {
    QCheckBox *c = new QCheckBox(tr("Show desktop notifications"), this);
    QSettings settings;
    bool checked = settings.value("showDesktopNotification").toBool();
    c->setChecked(checked);
    c->setToolTip(tr("Show desktop notifications"));
    m_layout->addWidget(c);

    connect(c, &QCheckBox::clicked, this,
            &GeneneralConf::showDesktopNotificationChanged);
}
