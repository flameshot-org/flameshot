// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#pragma once

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QVBoxLayout;

class PinHistoryConf : public QWidget
{
    Q_OBJECT
public:
    explicit PinHistoryConf(QWidget* parent = nullptr);

public slots:
    void updateComponents();

private slots:
    void enabledChanged(bool checked);
    void maxEntriesChanged(int value);
    void maxAgeChanged(int index);
    void revealStorage();
    void clearNow();

private:
    void initUi();
    void refreshStats();

    QVBoxLayout* m_layout;
    QLabel* m_unavailableBanner;
    QCheckBox* m_enabled;
    QSpinBox* m_maxEntries;
    QComboBox* m_maxAge;
    QLabel* m_pathLabel;
    QLabel* m_statsLabel;
    QPushButton* m_revealButton;
    QPushButton* m_clearButton;
};
