#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include "button.h"
#include <QVector>

class Button;
class QRect;
class QPoint;

class ButtonHandler {
public:
    ButtonHandler(const QVector<Button*>&);
    ButtonHandler();

    void hide();
    void show();

    bool isVisible() const;
    size_t size() const;

    void updatePosition(const QRect &selection, const QRect &limits);
    void setButtons(QVector<Button*>);
private:
    QVector<QPoint> getHPoints(const QPoint &center, const int elements) const;
    QVector<QPoint> getVPoints(const QPoint &center, const int elements) const;

    QVector<Button*> m_vectorButtons;
    int m_distance;
};

#endif // BUTTONHANDLER_H
