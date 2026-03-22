// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2020 Yurii Puchkov at Namecheap & Contributors

#pragma once

#include "utils/confighandler.h"

#include <QStringList>
#include <QVector>
#include <QWidget>

class SetShortcutDialog;
class QCheckBox;
class QTableWidget;
class QVBoxLayout;

class ShortcutsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShortcutsWidget(QWidget* parent = nullptr);

private:
    void initInfoTable();
#if (defined(Q_OS_MACOS))
    const QString& nativeOSHotKeyText(const QString& text);
#endif

private slots:
    void populateInfoTable();
    void onShortcutCellClicked(int, int);

private:
#if (defined(Q_OS_MACOS))
    QString m_res;
#endif
    ConfigHandler m_config;
    QTableWidget* m_table;
    QVBoxLayout* m_layout;
    QList<QStringList> m_shortcuts;

    void loadShortcuts();
    void appendShortcut(const QString& shortcutName,
                        const QString& description);
#if defined(Q_OS_WIN)
    void checkPrintScreenForcesSnipping();
    bool isPrintScreenKeyForSnippingDisabled();
    bool disablePrintScreenKeyForSnipping();

    void initMsScreenclipCheckbox();
    bool isMsScreenclipRegistered();
    bool registerMsScreenclip();
    bool unregisterMsScreenclip();
    QCheckBox* m_registerMsScreenclip;
#endif
};
