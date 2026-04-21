// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#include "pinhistorydialog.h"

#include "pinhistorymodel.h"
#include "utils/confighandler.h"
#include "utils/pinhistoryentry.h"
#include "utils/pinhistorymanager.h"
#include "utils/pinhistorystore.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

namespace {
constexpr int kTileWidth = 180;
constexpr int kTileHeight = 180;
} // namespace

PinHistoryDialog::PinHistoryDialog(PinHistoryManager* manager, QWidget* parent)
  : QDialog(parent)
  , m_manager(manager)
{
    setWindowTitle(tr("Pin history"));
    setMinimumSize(800, 500);
    setAttribute(Qt::WA_DeleteOnClose);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_toolbar = new QToolBar(this);
    m_sortAction = m_toolbar->addAction(tr("Newest first"));
    m_sortAction->setCheckable(true);
    m_sortAction->setChecked(true);
    connect(
      m_sortAction, &QAction::toggled, this, &PinHistoryDialog::toggleSort);

    m_toolbar->addSeparator();
    m_clearAction = m_toolbar->addAction(tr("Clear all"));
    connect(
      m_clearAction, &QAction::triggered, this, &PinHistoryDialog::clearAll);

    layout->addWidget(m_toolbar);

    m_model = new PinHistoryModel(m_manager->store(), this);

    m_view = new QListView(this);
    m_view->setModel(m_model);
    m_view->setViewMode(QListView::IconMode);
    m_view->setUniformItemSizes(true);
    m_view->setResizeMode(QListView::Adjust);
    m_view->setMovement(QListView::Static);
    m_view->setGridSize(QSize(kTileWidth, kTileHeight));
    m_view->setIconSize(QSize(164, 120));
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_view->setWordWrap(true);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_view,
            &QListView::customContextMenuRequested,
            this,
            &PinHistoryDialog::onContextMenu);
    connect(m_view,
            &QListView::doubleClicked,
            this,
            [this](const QModelIndex&) { restoreSelected(); });

    layout->addWidget(m_view);

    m_emptyLabel =
      new QLabel(tr("No pins yet. Dismissed pins will appear here."), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setEnabled(false);
    layout->addWidget(m_emptyLabel);

    connect(
      m_manager, &PinHistoryManager::changed, this, &PinHistoryDialog::refresh);

    m_manager->applyRetention();
    m_model->refresh();
    updateEmptyState();
}

void PinHistoryDialog::refresh()
{
    m_model->refresh();
    updateEmptyState();
}

void PinHistoryDialog::updateEmptyState()
{
    const bool empty = m_model->rowCount() == 0;
    m_view->setVisible(!empty);
    m_emptyLabel->setVisible(empty);
    m_clearAction->setEnabled(!empty);
}

QStringList PinHistoryDialog::selectedIds() const
{
    QStringList ids;
    const QList<PinHistoryEntry> entries =
      m_model->entriesAt(m_view->selectionModel()->selectedIndexes());
    ids.reserve(entries.size());
    for (const auto& e : entries) {
        ids.append(e.id);
    }
    return ids;
}

void PinHistoryDialog::onContextMenu(const QPoint& pos)
{
    const QModelIndex indexAtPos = m_view->indexAt(pos);
    if (!indexAtPos.isValid()) {
        return;
    }
    if (!m_view->selectionModel()->isSelected(indexAtPos)) {
        m_view->setCurrentIndex(indexAtPos);
    }
    const int selectionCount =
      m_view->selectionModel()->selectedIndexes().size();

    QMenu menu(this);
    QAction* restoreAction = menu.addAction(tr("Restore as pin"));
    QAction* copyAction = menu.addAction(tr("Copy to clipboard"));
    QAction* saveAction = menu.addAction(tr("Save as…"));
    QAction* revealAction = menu.addAction(tr("Reveal in file manager"));
    menu.addSeparator();
    QAction* removeAction = menu.addAction(tr("Delete"));

    copyAction->setEnabled(selectionCount == 1);
    restoreAction->setEnabled(selectionCount == 1);
    saveAction->setEnabled(selectionCount == 1);
    revealAction->setEnabled(selectionCount == 1);

    QAction* chosen = menu.exec(m_view->viewport()->mapToGlobal(pos));
    if (chosen == restoreAction) {
        restoreSelected();
    } else if (chosen == copyAction) {
        copySelected();
    } else if (chosen == saveAction) {
        saveSelected();
    } else if (chosen == revealAction) {
        revealSelected();
    } else if (chosen == removeAction) {
        removeSelected();
    }
}

void PinHistoryDialog::restoreSelected()
{
    const QStringList ids = selectedIds();
    if (ids.size() != 1) {
        return;
    }
    m_manager->restore(ids.first());
}

void PinHistoryDialog::copySelected()
{
    const QStringList ids = selectedIds();
    if (ids.size() != 1) {
        return;
    }
    const QPixmap p = m_manager->store()->loadPixmap(ids.first());
    if (p.isNull()) {
        return;
    }
    QApplication::clipboard()->setPixmap(p);
}

void PinHistoryDialog::saveSelected()
{
    const QStringList ids = selectedIds();
    if (ids.size() != 1) {
        return;
    }
    const PinHistoryEntry entry = m_manager->store()->entry(ids.first());
    if (!entry.isValid()) {
        return;
    }
    const QString defaultName = QStringLiteral("flameshot-pin-%1.png")
                                  .arg(entry.dismissedAt.toLocalTime().toString(
                                    QStringLiteral("yyyyMMdd-HHmmss")));
    const QString path = QFileDialog::getSaveFileName(
      this,
      tr("Save pin as…"),
      defaultName,
      tr("PNG image (*.png);;JPEG image (*.jpg);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    const QPixmap p = m_manager->store()->loadPixmap(entry.id);
    if (p.isNull()) {
        return;
    }
    p.save(path);
}

void PinHistoryDialog::revealSelected()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_manager->store()->path()));
}

void PinHistoryDialog::removeSelected()
{
    const QStringList ids = selectedIds();
    if (ids.isEmpty()) {
        return;
    }
    if (ConfigHandler().historyConfirmationToDelete()) {
        const QMessageBox::StandardButton answer = QMessageBox::warning(
          this,
          tr("Delete pins from history?"),
          tr("Delete %n pin(s) from history?", "", ids.size()),
          QMessageBox::Yes | QMessageBox::Cancel,
          QMessageBox::Cancel);
        if (answer != QMessageBox::Yes) {
            return;
        }
    }
    m_manager->store()->removeMany(ids);
    refresh();
}

void PinHistoryDialog::clearAll()
{
    if (m_model->rowCount() == 0) {
        return;
    }
    const QMessageBox::StandardButton answer = QMessageBox::warning(
      this,
      tr("Clear all pins?"),
      tr("Delete all %n pin(s) from history? This cannot be undone.",
         "",
         m_model->rowCount()),
      QMessageBox::Yes | QMessageBox::Cancel,
      QMessageBox::Cancel);
    if (answer != QMessageBox::Yes) {
        return;
    }
    m_manager->store()->clear();
    refresh();
}

void PinHistoryDialog::toggleSort()
{
    m_model->setNewestFirst(m_sortAction->isChecked());
    m_sortAction->setText(m_sortAction->isChecked() ? tr("Newest first")
                                                    : tr("Oldest first"));
}

void PinHistoryDialog::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            restoreSelected();
            return;
        case Qt::Key_Space:
            copySelected();
            return;
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            removeSelected();
            return;
        default:
            break;
    }
    QDialog::keyPressEvent(event);
}
