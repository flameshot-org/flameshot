// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#include "pinhistoryconf.h"

#include "utils/confighandler.h"
#include "utils/pinhistorystore.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QUrl>
#include <QVBoxLayout>

namespace {

struct AgeOption
{
    int days;
    const char* label;
};

const std::array<AgeOption, 5> kAgeOptions{ {
  { 0, QT_TRANSLATE_NOOP("PinHistoryConf", "Never") },
  { 7, QT_TRANSLATE_NOOP("PinHistoryConf", "7 days") },
  { 30, QT_TRANSLATE_NOOP("PinHistoryConf", "30 days") },
  { 90, QT_TRANSLATE_NOOP("PinHistoryConf", "90 days") },
  { 365, QT_TRANSLATE_NOOP("PinHistoryConf", "1 year") },
} };

QString formatBytes(qint64 bytes)
{
    constexpr double kKiB = 1024.0;
    constexpr double kMiB = kKiB * 1024.0;
    constexpr double kGiB = kMiB * 1024.0;
    if (bytes < kKiB) {
        return QStringLiteral("%1 B").arg(bytes);
    }
    if (bytes < kMiB) {
        return QStringLiteral("%1 KB").arg(bytes / kKiB, 0, 'f', 1);
    }
    if (bytes < kGiB) {
        return QStringLiteral("%1 MB").arg(bytes / kMiB, 0, 'f', 1);
    }
    return QStringLiteral("%1 GB").arg(bytes / kGiB, 0, 'f', 2);
}

} // namespace

PinHistoryConf::PinHistoryConf(QWidget* parent)
  : QWidget(parent)
{
    initUi();
    updateComponents();
}

void PinHistoryConf::initUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);

    m_unavailableBanner = new QLabel(this);
    m_unavailableBanner->setWordWrap(true);
    m_unavailableBanner->setStyleSheet(QStringLiteral(
      "QLabel { color: palette(highlight); font-weight: bold; }"));
    m_unavailableBanner->hide();
    m_layout->addWidget(m_unavailableBanner);

    m_enabled = new QCheckBox(tr("Enable pin history"), this);
    m_enabled->setToolTip(
      tr("When disabled, dismissed pins are not retained. Existing history is "
         "kept and will be reaccessible if you re-enable the feature."));
    connect(
      m_enabled, &QCheckBox::toggled, this, &PinHistoryConf::enabledChanged);
    m_layout->addWidget(m_enabled);

    auto* retentionGroup = new QGroupBox(tr("Retention"), this);
    auto* retentionLayout = new QVBoxLayout(retentionGroup);

    auto* entriesRow = new QHBoxLayout();
    entriesRow->addWidget(new QLabel(tr("Maximum entries:"), retentionGroup));
    m_maxEntries = new QSpinBox(retentionGroup);
    m_maxEntries->setRange(1, 1000);
    connect(m_maxEntries,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &PinHistoryConf::maxEntriesChanged);
    entriesRow->addWidget(m_maxEntries);
    entriesRow->addStretch();
    retentionLayout->addLayout(entriesRow);

    auto* ageRow = new QHBoxLayout();
    ageRow->addWidget(new QLabel(tr("Maximum age:"), retentionGroup));
    m_maxAge = new QComboBox(retentionGroup);
    for (const auto& opt : kAgeOptions) {
        m_maxAge->addItem(tr(opt.label), opt.days);
    }
    connect(m_maxAge,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PinHistoryConf::maxAgeChanged);
    ageRow->addWidget(m_maxAge);
    ageRow->addStretch();
    retentionLayout->addLayout(ageRow);

    m_layout->addWidget(retentionGroup);

    auto* storageGroup = new QGroupBox(tr("Storage"), this);
    auto* storageLayout = new QVBoxLayout(storageGroup);

    m_pathLabel = new QLabel(storageGroup);
    m_pathLabel->setWordWrap(true);
    m_pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    storageLayout->addWidget(m_pathLabel);

    m_statsLabel = new QLabel(storageGroup);
    storageLayout->addWidget(m_statsLabel);

    auto* buttonRow = new QHBoxLayout();
    m_revealButton =
      new QPushButton(tr("Reveal in file manager"), storageGroup);
    connect(m_revealButton,
            &QPushButton::clicked,
            this,
            &PinHistoryConf::revealStorage);
    buttonRow->addWidget(m_revealButton);

    m_clearButton = new QPushButton(tr("Clear history now"), storageGroup);
    connect(
      m_clearButton, &QPushButton::clicked, this, &PinHistoryConf::clearNow);
    buttonRow->addWidget(m_clearButton);
    buttonRow->addStretch();
    storageLayout->addLayout(buttonRow);

    m_layout->addWidget(storageGroup);
    m_layout->addStretch();
}

void PinHistoryConf::updateComponents()
{
    ConfigHandler cfg;
    const QSignalBlocker b1(m_enabled);
    const QSignalBlocker b2(m_maxEntries);
    const QSignalBlocker b3(m_maxAge);

    m_enabled->setChecked(cfg.pinHistoryEnabled());
    m_maxEntries->setValue(cfg.pinHistoryMaxEntries());

    const int currentAge = cfg.pinHistoryMaxAgeDays();
    int idx = 0;
    for (int i = 0; i < static_cast<int>(kAgeOptions.size()); ++i) {
        if (kAgeOptions[i].days == currentAge) {
            idx = i;
            break;
        }
    }
    m_maxAge->setCurrentIndex(idx);

    refreshStats();
}

void PinHistoryConf::refreshStats()
{
    PinHistoryStore store;
    m_pathLabel->setText(tr("Location: %1").arg(store.path()));
    const int count = store.entries().size();
    const qint64 bytes = store.diskUsage();
    m_statsLabel->setText(
      tr("%1 entries · %2").arg(count).arg(formatBytes(bytes)));

    const bool available = store.isAvailable();
    m_revealButton->setEnabled(available);
    m_clearButton->setEnabled(available && count > 0);
    m_enabled->setEnabled(available);
    m_maxEntries->setEnabled(available);
    m_maxAge->setEnabled(available);

    if (available) {
        m_unavailableBanner->hide();
    } else {
        m_unavailableBanner->setText(
          tr("Pin history storage is not writable (%1). The feature is "
             "disabled for this session.")
            .arg(store.path()));
        m_unavailableBanner->show();
    }
}

void PinHistoryConf::enabledChanged(bool checked)
{
    ConfigHandler().setPinHistoryEnabled(checked);
}

void PinHistoryConf::maxEntriesChanged(int value)
{
    ConfigHandler().setPinHistoryMaxEntries(value);
}

void PinHistoryConf::maxAgeChanged(int index)
{
    if (index < 0 || index >= static_cast<int>(kAgeOptions.size())) {
        return;
    }
    ConfigHandler().setPinHistoryMaxAgeDays(kAgeOptions[index].days);
}

void PinHistoryConf::revealStorage()
{
    PinHistoryStore store;
    QDesktopServices::openUrl(QUrl::fromLocalFile(store.path()));
}

void PinHistoryConf::clearNow()
{
    const QMessageBox::StandardButton answer =
      QMessageBox::warning(this,
                           tr("Clear pin history?"),
                           tr("This will permanently delete all retained pins. "
                              "This cannot be undone."),
                           QMessageBox::Yes | QMessageBox::Cancel,
                           QMessageBox::Cancel);
    if (answer != QMessageBox::Yes) {
        return;
    }
    PinHistoryStore store;
    store.clear();
    refreshStats();
}
