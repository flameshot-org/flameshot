#ifndef HOTKEYSCONFIG_H
#define HOTKEYSCONFIG_H

#include "src/utils/confighandler.h"
#include <QWidget>
#include <QVector>
#include <QStringList>


class SetShortcutDialog;
class QTableWidget;
class QVBoxLayout;

class ShortcutsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShortcutsWidget(QWidget *parent = nullptr);
    const QVector<QStringList> &shortcuts();

private:
    void initInfoTable();

private slots:
    void slotShortcutCellClicked(int, int);

private:
    ConfigHandler m_config;
    QTableWidget *m_table;
    QVBoxLayout *m_layout;
    QVector<QStringList> m_shortcuts;
};

#endif // HOTKEYSCONFIG_H
