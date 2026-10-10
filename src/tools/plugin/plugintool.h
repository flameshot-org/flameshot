// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "plugins/actionplugin.h"
#include "tools/abstractactiontool.h"

class PluginActionTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit PluginActionTool(const ActionPlugin& plugin,
                              QObject* parent = nullptr);

    bool closeOnButtonPressed() const override;
    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    CaptureTool::Type type() const override;
    QString description() const override;
    CaptureTool* copy(QObject* parent = nullptr) override;

public slots:
    void pressed(CaptureContext& context) override;

private:
    ActionPlugin m_plugin;
};
