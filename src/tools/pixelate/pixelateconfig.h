#pragma once

#include <QCheckBox>
#include <QVBoxLayout>
#include <QWidget>

class PixelateConfig : public QWidget
{
    Q_OBJECT
public:
    explicit PixelateConfig(QWidget* parent = nullptr);
    void setInvertSelection(bool invert);

signals:
    void toggleInvertSelection(bool invert);

private:
    QVBoxLayout* m_layout;
    QCheckBox* m_invertSelectionCheckBox;
};
