// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2020 Yurii Puchkov at Namecheap & Contributors

#ifndef HOTKEYSCONFIG_H
#define HOTKEYSCONFIG_H

#include "src/utils/confighandler.h"
#include <QStringList>
#include <QVector>
#include <QWidget>

class SetShortcutDialog;
class QTableWidget;
class QVBoxLayout;

class ShortcutsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShortcutsWidget(QWidget* parent = nullptr);
    const QVector<QStringList>& shortcuts();

private:
    void initInfoTable();
#if (defined(Q_OS_MAC) || defined(Q_OS_MAC64) || defined(Q_OS_MACOS) ||        \
     defined(Q_OS_MACX))
    const QString& nativeOSHotKeyText(const QString& text);
#endif

private slots:
    void slotShortcutCellClicked(int, int);

private:
#if (defined(Q_OS_MAC) || defined(Q_OS_MAC64) || defined(Q_OS_MACOS) ||        \
     defined(Q_OS_MACX))
    QString m_res;
#endif
    ConfigHandler m_config;
    QTableWidget* m_table;
    QVBoxLayout* m_layout;
    QVector<QStringList> m_shortcuts;
};

#endif // HOTKEYSCONFIG_H
