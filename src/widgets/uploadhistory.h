#ifndef UPLOADHISTORY_H
#define UPLOADHISTORY_H

#include "history.h"
#include <QWidget>
#include <qpushbutton.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class UploadHistory;
}
QT_END_NAMESPACE

void clearHistoryLayout(QLayout* layout);
void scaleThumbnail(QPixmap& input);

class UploadHistory : public QWidget
{
    Q_OBJECT
public:
    explicit UploadHistory(QWidget* parent = nullptr);
    ~UploadHistory();

    void loadHistory();
    void loadNextBatch(); // Load the next batch of images
public slots:

private:
    QList<QString> secondaryStorage; // Secondary storage for older screenshots
    void setEmptyMessage();
    QPushButton* loadMoreButton; // Attribute for the "Load More" button
    void addLine(QString const&, QString const&);
    QList<QString> historyFiles; // Store all history file paths
    int currentBatchStartIndex;  // Track the starting index for the next batch
    Ui::UploadHistory* ui;
    History history;
};
#endif // UPLOADHISTORY_H
