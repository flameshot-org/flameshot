// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/capturetool.h"
#include <QPointer>
#include <QWidget>

class QVBoxLayout;
class QPropertyAnimation;
class QScrollArea;
class QPushButton;
class QListWidget;
class CaptureTool;
class QPushButton;
class CaptureWidget;

class UtilityPanel : public QWidget
{
    Q_OBJECT
public:
    explicit UtilityPanel(CaptureWidget* captureWidget);

    QWidget* toolWidget() const;
    void setToolWidget(QWidget* w);
    void clearToolWidget();
    void pushWidget(QWidget* w);
    void hide();
    void show();
    void fillCaptureTools(
      QList<QPointer<CaptureTool>> captureToolObjectsHistory);
    void setActiveLayer(int index);
    int activeLayerIndex();

signals:
    void layerChanged(int layer);

public slots:
    void toggle();
    void slotButtonDelete(bool clicked);
    void slotCaptureToolsCurrentRowChanged(int currentRow);

private:
    void initInternalPanel();

    QPointer<QWidget> m_toolWidget;
    QScrollArea* m_internalPanel;
    QVBoxLayout* m_upLayout;
    QVBoxLayout* m_bottomLayout;
    QVBoxLayout* m_layout;
    QPropertyAnimation* m_showAnimation;
    QPropertyAnimation* m_hideAnimation;
    QVBoxLayout* m_layersLayout;
    QListWidget* m_captureTools;
    QPushButton* m_buttonDelete;
    CaptureWidget* m_captureWidget;
};
