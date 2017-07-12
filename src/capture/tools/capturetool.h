#ifndef CAPTURETOOL_H
#define CAPTURETOOL_H

#include <QObject>
#include <QVector>

class CaptureTool : public QObject
{
    Q_OBJECT
public:
    enum toolType{WORKER, PATH_DRAWER, LINE_DRAWER};

    explicit CaptureTool(QObject *parent = nullptr);

    virtual int getID() = 0;
    virtual bool isCheckable() = 0;
    virtual void drawChanges(QPixmap &pm, const QVector<QPoint> &points) = 0;
    virtual toolType getToolType() = 0;

signals:
    void requestAction();

public slots:
};

#endif // CAPTURETOOL_H
