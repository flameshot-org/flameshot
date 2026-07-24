#pragma once

#include <QWidget>

struct HistoryFileName;

QT_BEGIN_NAMESPACE
namespace Ui {
class UploadLineItem;
}
QT_END_NAMESPACE

class UploadLineItem : public QWidget
{
    Q_OBJECT
public:
    // packedFileName is the bare packed history entry name, which is all this
    // item needs it for: handing it to History for removal. The displayed
    // timestamp is resolved by the caller.
    UploadLineItem(QWidget* parent,
                   QPixmap const& preview,
                   QString const& timestamp,
                   QString const& url,
                   QString const& packedFileName,
                   HistoryFileName const& unpackFileName);
    ~UploadLineItem();

signals:
    void requestedDeletion();

private:
    Ui::UploadLineItem* ui;
};
