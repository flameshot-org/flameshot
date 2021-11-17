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
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileSystemWatcher>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QSizePolicy>
#include <QTabBar>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

// ConfigWindow contains the menus where you can configure the application

ConfigWindow::ConfigWindow(QWidget* parent)
  : QWidget(parent)
{
    // We wrap QTabWidget in a QWidget because of a Qt bug
    auto layout = new QVBoxLayout(this);
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->tabBar()->setUsesScrollButtons(false);
    layout->addWidget(m_tabWidget);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
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
    m_visualsTab = new QWidget();
    QVBoxLayout* visualsLayout = new QVBoxLayout(m_visualsTab);
    m_visualsTab->setLayout(visualsLayout);
    visualsLayout->addWidget(m_visuals);
    m_tabWidget->addTab(
      m_visualsTab, QIcon(modifier + "graphics.svg"), tr("Interface"));

    // filename
    m_filenameEditor = new FileNameEditor();
    m_filenameEditorTab = new QWidget();
    QVBoxLayout* filenameEditorLayout = new QVBoxLayout(m_filenameEditorTab);
    m_filenameEditorTab->setLayout(filenameEditorLayout);
    filenameEditorLayout->addWidget(m_filenameEditor);
    m_tabWidget->addTab(m_filenameEditorTab,
                        QIcon(modifier + "name_edition.svg"),
                        tr("Filename Editor"));

    // general
    m_generalConfig = new GeneralConf();
    m_generalConfigTab = new QWidget();
    QVBoxLayout* generalConfigLayout = new QVBoxLayout(m_generalConfigTab);
    m_generalConfigTab->setLayout(generalConfigLayout);
    generalConfigLayout->addWidget(m_generalConfig);
    m_tabWidget->addTab(
      m_generalConfigTab, QIcon(modifier + "config.svg"), tr("General"));

    // shortcuts
    m_shortcuts = new ShortcutsWidget();
    m_shortcutsTab = new QWidget();
    QVBoxLayout* shortcutsLayout = new QVBoxLayout(m_shortcutsTab);
    m_shortcutsTab->setLayout(shortcutsLayout);
    shortcutsLayout->addWidget(m_shortcuts);
    m_tabWidget->addTab(
      m_shortcutsTab, QIcon(modifier + "shortcut.svg"), tr("Shortcuts"));

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
    initErrorIndicator(m_visualsTab, m_visuals);
    initErrorIndicator(m_filenameEditorTab, m_filenameEditor);
    initErrorIndicator(m_generalConfigTab, m_generalConfig);
    initErrorIndicator(m_shortcutsTab, m_shortcuts);
}

void ConfigWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        close();
    }
}

void ConfigWindow::initErrorIndicator(QWidget* tab, QWidget* widget)
{
    QLabel* label = new QLabel(tab);
    QPushButton* btnShowErrors = new QPushButton("Show errors", tab);
    QHBoxLayout* btnLayout = new QHBoxLayout(tab);

    // Set up label
    label->setText(tr(
      "<b>Configuration file has errors. Resolve them before continuing.</b>"));
    label->setStyleSheet(QStringLiteral(":disabled { color: %1; }")
                           .arg(qApp->palette().color(QPalette::Text).name()));
    label->setVisible(ConfigHandler().hasError());

    // Set up "Show errors" button
    btnShowErrors->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    btnLayout->addWidget(btnShowErrors);
    btnShowErrors->setVisible(ConfigHandler().hasError());

    widget->setEnabled(!ConfigHandler().hasError());

    // Add label and button to the parent widget's layout
    QBoxLayout* layout = static_cast<QBoxLayout*>(tab->layout());
    if (layout != nullptr) {
        layout->insertWidget(0, label);
        layout->insertLayout(1, btnLayout);
    } else {
        widget->layout()->addWidget(label);
        widget->layout()->addWidget(btnShowErrors);
    }

    // Sigslots
    connect(ConfigHandler::getInstance(), &ConfigHandler::error, widget, [=]() {
        widget->setEnabled(false);
        label->show();
        btnShowErrors->show();
    });
    connect(ConfigHandler::getInstance(),
            &ConfigHandler::errorResolved,
            widget,
            [=]() {
                widget->setEnabled(true);
                label->hide();
                btnShowErrors->hide();
            });
    connect(btnShowErrors, &QPushButton::clicked, this, [this]() {
        // Generate error log message
        QString str;
        QTextStream stream(&str);
        ConfigHandler().checkForErrors(&stream);

        // Set up dialog
        QDialog dialog;
        dialog.setWindowTitle(QStringLiteral("Configuration errors"));
        dialog.setLayout(new QVBoxLayout(&dialog));

        // Add text display
        QTextEdit* textDisplay = new QTextEdit(&dialog);
        textDisplay->setPlainText(str);
        textDisplay->setReadOnly(true);
        dialog.layout()->addWidget(textDisplay);

        // Add Ok button
        using BBox = QDialogButtonBox;
        BBox* buttons = new BBox(BBox::Ok);
        dialog.layout()->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::clicked, this, [&dialog]() {
            dialog.close();
        });

        dialog.show();

        qApp->processEvents();
        QPoint center = dialog.geometry().center();
        QRect dialogRect(0, 0, 600, 400);
        dialogRect.moveCenter(center);
        dialog.setGeometry(dialogRect);

        dialog.exec();
    });
}
