#ifndef GLOBALSHORTCUTFILTER_H
#define GLOBALSHORTCUTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>

class GlobalShortcutFilter : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
public:
    explicit GlobalShortcutFilter(QObject *parent = 0);

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

signals:
    void printPressed();

private:
    quint32 getNativeModifier(Qt::KeyboardModifiers modifiers);
    quint32 nativeKeycode(Qt::Key key);
    bool registerShortcut(quint32 nativeKey, quint32 nativeMods);
    bool unregisterShortcut(quint32 nativeKey, quint32 nativeMods);

};

#endif // GLOBALSHORTCUTFILTER_H
