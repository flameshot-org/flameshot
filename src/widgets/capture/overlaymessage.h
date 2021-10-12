#pragma once

#include <QLabel>
#include <QStack>

/**
 * @brief Overlay a message in capture mode.
 *
 * The message must be initialized by calling `init` before it can be used. That
 * can be done once per capture session. The class is a singleton.
 *
 * To change the active message call `push`. This will automatically show the
 * widget. Previous messages won't be forgotten and will be reactivated after
 * you call `pop`. You can show/hide the message using `setVisibility` (this
 * won't push/pop anything).
 *
 * @note You have to make sure that widgets pop the messages they pushed when
 * they are closed, to avoid potential bugs and resource leaks.
 */
class OverlayMessage : public QLabel
{
public:
    OverlayMessage() = delete;

    static void init(QWidget* parent, const QRect& targetArea);
    static void push(const QString& msg);
    static void pop();
    static void setVisibility(bool visible);
    static OverlayMessage* instance();

    static void pushKeyMap(const QList<QPair<QString, QString>>& map);
    static QString compileFromKeyMap(const QList<QPair<QString, QString>>& map);

private:
    QStack<QString> m_messageStack;
    QRect m_targetArea;
    QColor m_fillColor, m_textColor;
    static OverlayMessage* m_instance;

    OverlayMessage(QWidget* parent, const QRect& center);

    void paintEvent(QPaintEvent*) override;

    QRect boundingRect() const;
    void updateGeometry();
};
