#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#define HISTORYPIXMAP_MAX_PREVIEW_WIDTH 160
#define HISTORYPIXMAP_MAX_PREVIEW_HEIGHT 90

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QString>

class QLayout;
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
    void addLine(const QString &, const QString &);
    void removeItem(QLayout *, const QString &);
    void setEmptyMessage();

private:
    QVBoxLayout *m_pVBox;
    NotificationWidget *m_notification;
};

#endif // HISTORYWIDGET_H
