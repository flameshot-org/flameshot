#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>
#include <map>
#include "button.h"

class QWidget;
class QPropertyAnimation;

class Button : public QPushButton {
    Q_OBJECT

public:
    enum class Type {
        selectionIndicator,
        mouseVisibility,
        exit,
        copy,
        save,
        pencil,
        line,
        arrow,
        rectangle,
        circle,
        marker,
        text,
        colorPicker,
        undo,
        imageUploader,
        move,
        last
    };

    typedef std::map<Button::Type, const QString> typeData;
    static typeData typeTooltip;
    static typeData typeName;

    explicit Button(Type, QWidget *parent = 0);

    static QIcon getIcon(const Type);
    static size_t getButtonBaseSize();

    Type getButtonType() const;

    void animatedShow();

protected:
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

private:
    Button(QWidget *parent = 0);
    Type m_buttonType;

    QPropertyAnimation *emergeAnimation;

signals:
    void hovered();
    void mouseExited();
    void typeEmited(Type);

public slots:
};

#endif // BUTTON_H
