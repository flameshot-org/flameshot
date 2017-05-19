#ifndef UICOLORPICKER_H
#define UICOLORPICKER_H

#include <QWidget>

class UIcolorEditor : public QWidget {
    Q_OBJECT
public:
    explicit UIcolorEditor(QWidget *parent = 0);

private slots:
    void updateUIcolor();
    void updateLocalColor(QColor);

private:
    QColor m_uiColor;
};

#endif // UICOLORPICKER_H
