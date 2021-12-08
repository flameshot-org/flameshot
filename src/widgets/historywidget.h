#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QDialog>

class QString;
class QLayout;
class QVBoxLayout;
class NotificationWidget;
class ImgUploader;

class HistoryWidget : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryWidget(QWidget* parent = nullptr);
    ~HistoryWidget();

    void loadHistory();

private:
    void clearHistoryLayout(QLayout* layout);

    void addLine(const QString&, const QString&);
    void setEmptyMessage();
    void removeItem(QLayout* pl,
                    const QString& s3FileName,
                    const QString& deleteToken);
    void removeLayoutItem(QLayout* pl);
    void removeCacheFile(const QString& fullFileName);

private:
    QVBoxLayout* m_pVBox;
    NotificationWidget* m_notification;
};

#endif // HISTORYWIDGET_H
