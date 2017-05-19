#ifndef BUTTONLISTVIEW_H
#define BUTTONLISTVIEW_H

#include <QListWidget>

class ButtonListView : public QListWidget {
public:
    ButtonListView(QWidget *parent= 0);

private slots:
    void updateActiveButtons(QListWidgetItem *);
    void reverseItemCheck(QListWidgetItem *);

protected:
    void initButtonList();

private:
    QList<int> m_listButtons;
};

#endif // BUTTONLISTVIEW_H
