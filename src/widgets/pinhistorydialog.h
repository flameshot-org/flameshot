// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#pragma once

#include <QDialog>

class PinHistoryManager;
class PinHistoryModel;
class QAction;
class QListView;
class QLabel;
class QToolBar;

class PinHistoryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PinHistoryDialog(PinHistoryManager* manager,
                              QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onContextMenu(const QPoint& pos);
    void restoreSelected();
    void copySelected();
    void saveSelected();
    void revealSelected();
    void removeSelected();
    void clearAll();
    void toggleSort();
    void refresh();

private:
    QStringList selectedIds() const;
    void updateEmptyState();

    PinHistoryManager* m_manager;
    PinHistoryModel* m_model;
    QListView* m_view;
    QLabel* m_emptyLabel;
    QToolBar* m_toolbar;
    QAction* m_sortAction;
    QAction* m_clearAction;
};
