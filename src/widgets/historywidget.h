#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#define HISTORYWIDGET_MAX_PREVIEW_HEIGHT 120

#include <QObject>
#include <QWidget>
#include <QDialog>

class QVBoxLayout;
class NotificationWidget;

class HistoryWidget : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryWidget(QWidget *parent = nullptr);

signals:

private:
    void loadHistory();

private:
    QVBoxLayout *m_pVBox;
    NotificationWidget *m_notification;
};

#endif // HISTORYWIDGET_H
