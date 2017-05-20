#ifndef UICOLORPICKER_H
#define UICOLORPICKER_H

#include <QFrame>

class QVBoxLayout;
class QHBoxLayout;
class Button;

class UIcolorEditor : public QFrame {
    Q_OBJECT
public:
    explicit UIcolorEditor(QWidget *parent = 0);

private slots:
    void updateUIcolor();
    void updateLocalColor(QColor);

private:
    QColor m_uiColor;
    Button *m_button;

    QHBoxLayout *hLayout;
    QVBoxLayout *vLayout;

    void initColorWheel();
    void initButton();
};

#endif // UICOLORPICKER_H
