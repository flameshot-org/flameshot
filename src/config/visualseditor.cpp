// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "visualseditor.h"
#include "config/buttonlistview.h"
#include "config/colorpickereditor.h"
#include "config/extendedslider.h"
#include "config/uicoloreditor.h"
#include "plugins/pluginmanager.h"
#include "utils/confighandler.h"

#include <QDirIterator>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

VisualsEditor::VisualsEditor(QWidget* parent)
  : QWidget(parent)
{
    m_layout = new QVBoxLayout();
    setLayout(m_layout);
    initWidgets();
}

void VisualsEditor::updateComponents()
{
    m_buttonList->updateComponents();
    m_colorEditor->updateComponents();
    int opacity = ConfigHandler().contrastOpacity();
    m_opacitySlider->setMapedValue(0, opacity, 255);
}

void VisualsEditor::initOpacitySlider()
{
    m_opacitySlider = new ExtendedSlider();
    m_opacitySlider->setFocusPolicy(Qt::NoFocus);
    m_opacitySlider->setOrientation(Qt::Horizontal);
    m_opacitySlider->setRange(0, 100);
    auto* localLayout = new QHBoxLayout();
    localLayout->addWidget(new QLabel(QStringLiteral("0%")));
    localLayout->addWidget(m_opacitySlider);
    localLayout->addWidget(new QLabel(QStringLiteral("100%")));

    auto* label = new QLabel();
    QString labelMsg = tr("Opacity of area outside selection:") + " %1%";
    ExtendedSlider* opacitySlider = m_opacitySlider;
    connect(m_opacitySlider,
            &ExtendedSlider::valueChanged,
            this,
            [labelMsg, label, opacitySlider](int val) {
                label->setText(labelMsg.arg(val));
                ConfigHandler().setContrastOpacity(
                  opacitySlider->mappedValue(0, 255));
            });
    m_layout->addWidget(label);
    m_layout->addLayout(localLayout);

    int opacity = ConfigHandler().contrastOpacity();
    m_opacitySlider->setMapedValue(0, opacity, 255);
}

void VisualsEditor::initWidgets()
{
    initTranslations();

    m_tabWidget = new QTabWidget();
    m_layout->addWidget(m_tabWidget);

    m_colorEditor = new UIcolorEditor();
    m_colorEditorTab = new QWidget();
    auto* colorEditorLayout = new QVBoxLayout(m_colorEditorTab);
    m_colorEditorTab->setLayout(colorEditorLayout);
    colorEditorLayout->addWidget(m_colorEditor);
    m_tabWidget->addTab(m_colorEditorTab, tr("UI Color Editor"));

    m_colorpickerEditor = new ColorPickerEditor();
    m_colorpickerEditorTab = new QWidget();
    auto* colorpickerEditorLayout = new QVBoxLayout(m_colorpickerEditorTab);
    colorpickerEditorLayout->addWidget(m_colorpickerEditor);
    m_tabWidget->addTab(m_colorpickerEditorTab, tr("Colorpicker Editor"));

    initOpacitySlider();

    auto* boxButtons = new QGroupBox();
    boxButtons->setTitle(tr("Button Selection"));
    auto* listLayout = new QVBoxLayout(boxButtons);
    m_buttonList = new ButtonListView();
    m_layout->addWidget(boxButtons);
    listLayout->addWidget(m_buttonList);

    auto* buttonActions = new QHBoxLayout();
    auto* setAllButtons = new QPushButton(tr("Select All"));
    connect(setAllButtons,
            &QPushButton::clicked,
            m_buttonList,
            &ButtonListView::selectAll);
    buttonActions->addWidget(setAllButtons);

    auto* installPlugin = new QPushButton(tr("Install Plugin…"));
    connect(installPlugin, &QPushButton::clicked, this, [this]() {
        const QString package = QFileDialog::getOpenFileName(
          this,
          tr("Install Flameshot Plugin"),
          {},
          tr("Flameshot plugins (*.flameshot-plugin);;All files (*)"));
        if (package.isEmpty()) {
            return;
        }
        const PluginManagerResult result = PluginManager::install(package);
        if (result.success) {
            m_buttonList->reload();
            QMessageBox::information(
              this, tr("Plugin installed"), result.message);
        } else {
            QMessageBox::warning(
              this, tr("Plugin installation failed"), result.message);
        }
    });
    buttonActions->addWidget(installPlugin);

    auto* removePlugin = new QPushButton(tr("Remove Plugin"));
    connect(removePlugin, &QPushButton::clicked, this, [this]() {
        const QString pluginId = m_buttonList->selectedExternalPluginId();
        if (pluginId.isEmpty()) {
            QMessageBox::information(
              this,
              tr("Remove Plugin"),
              tr("Select an installed plugin first."));
            return;
        }
        if (QMessageBox::question(
              this,
              tr("Remove Plugin"),
              tr("Remove plugin %1?").arg(pluginId)) != QMessageBox::Yes) {
            return;
        }
        const PluginManagerResult result = PluginManager::remove(pluginId);
        if (result.success) {
            m_buttonList->reload();
            QMessageBox::information(
              this, tr("Plugin removed"), result.message);
        } else {
            QMessageBox::warning(
              this, tr("Plugin removal failed"), result.message);
        }
    });
    buttonActions->addWidget(removePlugin);
    listLayout->addLayout(buttonActions);
}

void VisualsEditor::initTranslations()
{
    auto* localLayout = new QHBoxLayout();
    localLayout->addWidget(new QLabel(tr("UI language")));
    m_selectTranslation = new QComboBox(this);

    QStringList translations;
    QString tmpFilename;
    for (const QString& path : PathInfo::translationsPaths()) {
        QDirIterator it(path,
                        QStringList() << QStringLiteral("*.qm"),
                        QDir::NoDotAndDotDot | QDir::Files);
        while (it.hasNext()) {
            it.next();
            tmpFilename = it.fileName();

            if (tmpFilename.startsWith(
                  QStringLiteral("Internationalization_"))) {
                tmpFilename =
                  tmpFilename.remove(QStringLiteral("Internationalization_"))
                    .remove(QStringLiteral(".qm"));
                if (!translations.contains(tmpFilename)) {
                    translations << tmpFilename;
                }
            }
        }
    }
    translations.sort();
    translations.push_front(QStringLiteral("auto"));
    m_selectTranslation->addItems(translations);

    QString language = ConfigHandler().value("uiLanguage").toString();
    m_selectTranslation->setCurrentIndex(
      m_selectTranslation->findText(language));

    connect(m_selectTranslation,
            &QComboBox::currentTextChanged,
            this,
            [this](const QString& text) {
                ConfigHandler().setUiLanguage(text);
                // TODO: Retranslate UI without restart
                QMessageBox::information(
                  this,
                  tr("Configuration"),
                  tr("Flameshot must be restarted to apply these changes!"));
            });

    localLayout->addWidget(m_selectTranslation);
    localLayout->addStretch();
    m_layout->addLayout(localLayout);
}
