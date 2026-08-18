// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "tools/abstractactiontool.h"

// Saves the current capture straight to one of 3 pre-configured folders
// (General tab > Save Path > Location 1/2/3), with no save dialog - same
// no-dialog behavior as `flameshot gui -p <path>`, but bindable to its own
// hotkey from the Shortcuts tab like any other in-editor tool.
class SaveLocationTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit SaveLocationTool(int location, QObject* parent = nullptr);

    bool closeOnButtonPressed() const override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;

protected:
    CaptureTool::Type type() const override;

public slots:
    void pressed(CaptureContext& context) override;

private:
    int m_location;
};
