// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include "plugins/flameshotplugininterface.h"
#include "tools/abstractactiontool.h"

class SampleTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit SampleTool(QObject* parent = nullptr);

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    CaptureTool::Type type() const override;
    QString description() const override;
    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;
    void pressed(CaptureContext& context) override;
    bool closeOnButtonPressed() const override;
};

class SamplePlugin
  : public QObject
  , public FlameshotPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FlameshotPluginInterface_iid)
    Q_INTERFACES(FlameshotPluginInterface)

public:
    explicit SamplePlugin(QObject* parent = nullptr);

    QString pluginId() const override;
    QString pluginName() const override;
    QString pluginVersion() const override;
    QString pluginAuthor() const override;
    QString pluginDescription() const override;
    QIcon pluginIcon() const override;

    QList<CaptureTool*> createTools(QObject* parent = nullptr) override;
};
