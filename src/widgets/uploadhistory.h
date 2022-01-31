#ifndef UPLOADHISTORY_H
#define UPLOADHISTORY_H

#include <QWidget>

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

public slots:

private:
    void setEmptyMessage();
    void addLine(QString const&, QString const&);

    Ui::UploadHistory* ui;
};
#endif // UPLOADHISTORY_H