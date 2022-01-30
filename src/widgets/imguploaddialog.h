// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QDialog>

class QCheckBox;
class QLabel;
class QDialogButtonBox;
class QVBoxLayout;

class ImgUploadDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImgUploadDialog(QDialog* parent = nullptr);

private:
    QCheckBox* m_uploadWithoutConfirmation;
    QLabel* m_uploadLabel;
    QDialogButtonBox* buttonBox;
    QVBoxLayout* layout;
};
