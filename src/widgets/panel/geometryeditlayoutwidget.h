#pragma once

#include <QGroupBox>

class QSpinBox;

class GeometryEditLayoutWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit GeometryEditLayoutWidget(QWidget* parent = nullptr);
    void update(QRect const& geometry);
signals:
    void edited(QRect const& geometry);

private:
    QSpinBox* m_wEditor;
    QSpinBox* m_hEditor;
    QSpinBox* m_xEditor;
    QSpinBox* m_yEditor;
};
