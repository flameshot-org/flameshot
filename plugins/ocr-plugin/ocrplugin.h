// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include "plugins/flameshotplugininterface.h"
#include <QObject>

class OcrPlugin
  : public QObject
  , public FlameshotPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FlameshotPluginInterface_iid)
    Q_INTERFACES(FlameshotPluginInterface)

public:
    explicit OcrPlugin(QObject* parent = nullptr);
    ~OcrPlugin() override = default;

    QString pluginId() const override;
    QString pluginName() const override;
    QString pluginVersion() const override;
    QString pluginAuthor() const override;
    QString pluginDescription() const override;
    QIcon pluginIcon() const override;

    QList<CaptureTool*> createTools(QObject* parent = nullptr) override;
};
