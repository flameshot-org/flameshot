// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "configwindow.h"
#include "src/config/filenameeditor.h"
#include "src/config/generalconf.h"
#include "src/config/shortcutswidget.h"
#include "src/config/strftimechooserwidget.h"
#include "src/config/visualseditor.h"
#include "src/utils/colorutils.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include "src/utils/pathinfo.h"
#include <QApplication>
#include <QFileSystemWatcher>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QTabBar>
#include <QVBoxLayout>

// ConfigWindow contains the menus where you can configure the application

ConfigWindow::ConfigWindow(QWidget* parent)
  : QWidget(parent)
{
    // We wrap QTabWidget in a QWidget because of a Qt bug
    auto layout = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);
    m_tabs->tabBar()->setUsesScrollButtons(false);
    layout->addWidget(m_tabs);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(":img/app/flameshot.svg"));
    setWindowTitle(tr("Configuration"));

    connect(ConfigHandler::getInstance(),
            &ConfigHandler::fileChanged,
            this,
            &ConfigWindow::updateChildren);

    QColor background = this->palette().window().color();
    bool isDark = ColorUtils::colorIsDark(background);
    QString modifier =
      isDark ? PathInfo::whiteIconPath() : PathInfo::blackIconPath();

    // visuals
    m_visuals = new VisualsEditor();
    m_tabs->addTab(
      m_visuals, QIcon(modifier + "graphics.svg"), tr("Interface"));

    // filename
    m_filenameEditor = new FileNameEditor();
    m_tabs->addTab(m_filenameEditor,
                   QIcon(modifier + "name_edition.svg"),
                   tr("Filename Editor"));

    // general
    m_generalConfig = new GeneralConf();
    m_tabs->addTab(
      m_generalConfig, QIcon(modifier + "config.svg"), tr("General"));

    // shortcuts
    m_shortcuts = new ShortcutsWidget();
    m_tabs->addTab(
      m_shortcuts, QIcon(modifier + "shortcut.svg"), tr("Shortcuts"));

    // connect update sigslots
    connect(this,
            &ConfigWindow::updateChildren,
            m_filenameEditor,
            &FileNameEditor::updateComponents);
    connect(this,
            &ConfigWindow::updateChildren,
            m_visuals,
            &VisualsEditor::updateComponents);
    connect(this,
            &ConfigWindow::updateChildren,
            m_generalConfig,
            &GeneralConf::updateComponents);

    // Error indicator (this must come last)
    initErrorLabel(m_visuals);
    initErrorLabel(m_filenameEditor);
    initErrorLabel(m_generalConfig);
    initErrorLabel(m_shortcuts);
}

void ConfigWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        close();
    }
}

void ConfigWindow::initErrorLabel(QWidget* widget)
{
    QLabel* label = new QLabel(widget);
    label->setText(tr(
      "<b>Configuration file has errors. Resolve them before continuing.</b>"));
    label->setStyleSheet(QStringLiteral(":disabled { color: %1; }")
                           .arg(qApp->palette().color(QPalette::Text).name()));

    label->setVisible(ConfigHandler().hasError());
    widget->setEnabled(!ConfigHandler().hasError());

    QBoxLayout* layout = static_cast<QBoxLayout*>(widget->layout());
    if (layout != nullptr) {
        layout->insertWidget(0, label);
    } else {
        widget->layout()->addWidget(label);
    }

    // Sigslots
    connect(ConfigHandler::getInstance(), &ConfigHandler::error, widget, [=]() {
        widget->setEnabled(false);
        label->show();
    });
    connect(ConfigHandler::getInstance(),
            &ConfigHandler::errorResolved,
            widget,
            [=]() {
                widget->setEnabled(true);
                label->hide();
            });
}
