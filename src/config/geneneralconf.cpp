#include "geneneralconf.h"

#include <QVBoxLayout>
#include <QSettings>
#include <QCheckBox>

GeneneralConf::GeneneralConf(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel);

    m_layout = new QVBoxLayout(this);
    initHelpShow();
}

void GeneneralConf::showHelpChanged(bool checked) {
    QSettings().setValue("showHelp", checked);
}

void GeneneralConf::initHelpShow() {
    QCheckBox *c = new QCheckBox(tr("Show help message"), this);
    QSettings settings;
    bool checked = settings.value("showHelp").toBool();
    c->setChecked(checked);
    c->setToolTip(tr("Show the help message at the beginning "
                       "in the capture mode."));
    m_layout->addWidget(c);

    connect(c, &QCheckBox::clicked, this, &GeneneralConf::showHelpChanged);
}
