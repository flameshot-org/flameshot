#ifndef VISUALSEDITOR_H
#define VISUALSEDITOR_H

#include <QWidget>

class ExtendedSlider;
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
    void saveOpacity();

private:
    QVBoxLayout *m_layout;
    ButtonListView *m_buttonList;
    UIcolorEditor *m_colorEditor;
    ExtendedSlider *m_opacitySlider;

    void initWidgets();
    void initOpacitySlider();
};

#endif // VISUALSEDITOR_H
