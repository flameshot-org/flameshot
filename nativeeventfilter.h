#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>

class NativeEventFilter : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
public:
    explicit NativeEventFilter(QObject *parent = 0);

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

signals:
    void activated();

public slots:

private:
    void setShortcut();
};

#endif // NATIVEEVENTFILTER_H
