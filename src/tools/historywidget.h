#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QObject>
#include <QWidget>

class QVBoxLayout;

class HistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryWidget(QWidget *parent = nullptr);

signals:

private:
    void loadHistory();

private:
    QVBoxLayout *m_pVBox;
};

#endif // HISTORYWIDGET_H
