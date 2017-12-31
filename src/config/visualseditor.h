#ifndef VISUALSEDITOR_H
#define VISUALSEDITOR_H

#include <QWidget>

class QSlider;
class QVBoxLayout;
class ButtonListView;
class UIcolorEditor;

class VisualsEditor : public QWidget
{
    Q_OBJECT
public:
    explicit VisualsEditor(QWidget *parent = nullptr);

public slots:
    void updateComponents();

private slots:
    void updateOpacity(int);
    void saveOpacity();

private:
    int m_opacity; // Slider local value
    QVBoxLayout *m_layout;
    ButtonListView *m_buttonList;
    UIcolorEditor *m_colorEditor;
    QSlider *m_opacitySlider;

    void initWidgets();
    void initOpacitySlider();
};

#endif // VISUALSEDITOR_H
