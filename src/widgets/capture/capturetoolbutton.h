// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "capturebutton.h"
#include "tools/capturetool.h"

#include <QMap>
#include <QVector>

class QWidget;
class QPropertyAnimation;
struct ToolDescriptor;

class CaptureToolButton : public CaptureButton
{
    Q_OBJECT

public:
    explicit CaptureToolButton(const CaptureTool::Type,
                               QWidget* parent = nullptr);
    explicit CaptureToolButton(const ToolDescriptor& descriptor,
                               QWidget* parent = nullptr);
    ~CaptureToolButton();

    static const QList<CaptureTool::Type>& getIterableButtonTypes();
    static int getPriorityByButton(CaptureTool::Type);

    QString name() const;
    QString description() const;
    QIcon icon() const;
    CaptureTool* tool() const;
    QString toolId() const;

    void setColor(const QColor& c);
    void animatedShow();

protected:
    void mousePressEvent(QMouseEvent* e) override;

signals:
    void pressedButtonLeftClick(CaptureToolButton*);
    void pressedButtonRightClick(CaptureToolButton*);

private:
    CaptureToolButton(QWidget* parent = nullptr);
    CaptureTool::Type m_buttonType;
    QString m_toolId;
    QString m_shortcut;
    bool m_external;
    CaptureTool* m_tool;

    QPropertyAnimation* m_emergeAnimation;

    static QColor m_mainColor;

    void initButton();
    void updateIcon();
};
