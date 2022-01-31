#ifndef UPLOADLINEITEM_H
#define UPLOADLINEITEM_H

#include <QWidget>

struct HistoryFileName;

QT_BEGIN_NAMESPACE
namespace Ui {
class UploadLineItem;
}
QT_END_NAMESPACE

void removeCacheFile(QString const& fullFileName);

class UploadLineItem : public QWidget
{
    Q_OBJECT
public:
    UploadLineItem(QWidget* parent,
                   QPixmap const& preview,
                   QString const& timestamp,
                   QString const& url,
                   QString const& fullFileName,
                   HistoryFileName const& unpackFileName);
    ~UploadLineItem();

signals:
    void requestedDeletion();

private:
    Ui::UploadLineItem* ui;
};
#endif // UPLOADLINEITEM_H