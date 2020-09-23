#include "filepathconfiguration.h"
#include "src/config/strftimechooserwidget.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

FilePathConfiguration::FilePathConfiguration(QWidget* parent)
  : QWidget(parent)
{
    initWidgets();
    initLayout();
}

void FilePathConfiguration::initLayout()
{
    m_layout = new QVBoxLayout(this);

    m_layout->addWidget(new QLabel(tr("Screenshot path default:")));
    m_layout->addWidget(m_screenshotPathFixed);
    m_layout->addWidget(m_screenshotPathFixedDefault);
    m_layout->addStretch();
    QHBoxLayout* horizScreenshotButtonsLayout = new QHBoxLayout();
    horizScreenshotButtonsLayout->addStretch();
    horizScreenshotButtonsLayout->addWidget(m_screenshotPathFixedClear);
    horizScreenshotButtonsLayout->addWidget(m_screenshotPathFixedBrowse);
    m_layout->addLayout(horizScreenshotButtonsLayout);
}

void FilePathConfiguration::initWidgets()
{
    ConfigHandler config;

    // Screenshot path default
    m_screenshotPathFixed =
      new QCheckBox(tr("Use fixed path for screenshots to save"), this);
    m_screenshotPathFixed->setChecked(!config.savePathFixed().isEmpty());
    connect(m_screenshotPathFixed,
            SIGNAL(toggled(bool)),
            this,
            SLOT(sreenshotPathFixed()));
    m_screenshotPathFixedDefault = new QLineEdit(this);
    m_screenshotPathFixedDefault->setText(config.savePathFixed());
    m_screenshotPathFixedDefault->setDisabled(config.savePathFixed().isEmpty());
    m_screenshotPathFixedBrowse = new QPushButton(tr("Browse"), this);
    m_screenshotPathFixedBrowse->setDisabled(config.savePathFixed().isEmpty());
    connect(m_screenshotPathFixedBrowse,
            SIGNAL(released()),
            this,
            SLOT(screenshotPathFixedSet()));
    m_screenshotPathFixedClear = new QPushButton(tr("Clear"), this);
    m_screenshotPathFixedClear->setDisabled(config.savePathFixed().isEmpty());
    connect(m_screenshotPathFixedClear,
            SIGNAL(released()),
            this,
            SLOT(screenshotPathFixedClear()));
}

void FilePathConfiguration::sreenshotPathFixed()
{
    bool status = m_screenshotPathFixedDefault->isEnabled();
    m_screenshotPathFixedDefault->setEnabled(!status);
    m_screenshotPathFixedBrowse->setEnabled(!status);
    m_screenshotPathFixedClear->setEnabled(!status);
    screenshotPathFixedClear();
}

void FilePathConfiguration::screenshotPathFixedSet()
{
    QFileDialog* dirDialog =
      new QFileDialog(this, tr("Select default path for Screenshots"));
    dirDialog->setFileMode(QFileDialog::DirectoryOnly);
    dirDialog->setOption(QFileDialog::ShowDirsOnly, true);
    if (dirDialog->exec()) {
        QDir d = dirDialog->directory();
        QString absolutePath = d.absolutePath();
        m_screenshotPathFixedDefault->setText(absolutePath);
        ConfigHandler config;
        config.setSavePathFixed(absolutePath);
    }
}

void FilePathConfiguration::screenshotPathFixedClear()
{
    ConfigHandler config;
    m_screenshotPathFixedDefault->setText("");
    config.setSavePathFixed(m_screenshotPathFixedDefault->text());
}
