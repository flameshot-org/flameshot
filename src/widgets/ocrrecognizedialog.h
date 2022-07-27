// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Cavivie & Contributors

#pragma once

#include <QDialog>

class QCheckBox;
class QLabel;
class QDialogButtonBox;
class QVBoxLayout;

class OcrRecognizeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OcrRecognizeDialog(QDialog* parent = nullptr);

private:
    QCheckBox* m_ocrWithoutConfirmation;
    QLabel* m_ocrLabel;
    QDialogButtonBox* buttonBox;
    QVBoxLayout* layout;
};
