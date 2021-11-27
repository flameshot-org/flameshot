// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "cloudconf.h"
#include "src/core/controller.h"
#include "src/utils/confighandler.h"
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImageWriter>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QTextCodec>
#include <QVBoxLayout>

CloudConf::CloudConf(QWidget* parent)
  : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);

    initCredentialHolder();

    m_layout->addStretch();

    // this has to be at the end
    updateComponents();
}

void CloudConf::_updateComponents(bool allowEmptySavePath)
{
    ConfigHandler config;
    m_imgurUploadButton->setChecked(config.cloudImgur());
    m_droplrUploadButton->setChecked(config.cloudDroplr());

    if (config.cloudImgur()) {
        m_droplrUsernameEditor->setDisabled(true);
        m_droplrPasswordEditor->setDisabled(true);
    }
    m_droplrUsernameEditor->setText(config.droplrUsername());
    m_droplrPasswordEditor->setText(config.droplrPassword());
}

void CloudConf::updateComponents()
{
    _updateComponents(false);
}

void CloudConf::cloudUploadSetToImgur(bool checked)
{
    ConfigHandler().setCloudImgur(checked);
    ConfigHandler().setCloudDroplr(false);
    m_droplrUsernameEditor->setDisabled(true);
    m_droplrPasswordEditor->setDisabled(true);
}

void CloudConf::cloudUploadSetToDroplr(bool checked)
{
    ConfigHandler().setCloudImgur(false);
    ConfigHandler().setCloudDroplr(checked);
    m_droplrUsernameEditor->setDisabled(false);
    m_droplrPasswordEditor->setDisabled(false);
}

void CloudConf::setDroplrUsername(const QString& username)
{
    ConfigHandler().setDroplrUsername(username);
}

void CloudConf::setDroplrPassword(const QString& password)
{
    ConfigHandler().setDroplrPassword(password);
}

void CloudConf::initCredentialHolder()
{

    m_imgurUploadButton = new QRadioButton(tr("Upload to Imgur"), this);
    m_imgurUploadButton->setToolTip(tr("Upload image to Imgur Cloud"));
    m_layout->addWidget(m_imgurUploadButton);

    m_droplrUploadButton = new QRadioButton(tr("Upload to Droplr"), this);
    m_droplrUploadButton->setToolTip(tr("Upload image to Droplr Cloud"));
    m_layout->addWidget(m_droplrUploadButton);

    m_layout->addWidget(new QLabel(tr("Username/Email:")));
    m_droplrUsernameEditor = new QLineEdit(this);
    m_layout->addWidget(m_droplrUsernameEditor);

    m_layout->addWidget(new QLabel(tr("Password:")));
    m_droplrPasswordEditor = new QLineEdit(this);
    m_droplrPasswordEditor->setEchoMode(QLineEdit::Password);
    m_layout->addWidget(m_droplrPasswordEditor);

    connect(m_imgurUploadButton,
            &QRadioButton::clicked,
            this,
            &CloudConf::cloudUploadSetToImgur);
    connect(m_droplrUploadButton,
            &QRadioButton::clicked,
            this,
            &CloudConf::cloudUploadSetToDroplr);

    connect(m_droplrUsernameEditor,
            &QLineEdit::textChanged,
            this,
            &CloudConf::setDroplrUsername);

    connect(m_droplrPasswordEditor,
            &QLineEdit::textChanged,
            this,
            &CloudConf::setDroplrPassword);
}
