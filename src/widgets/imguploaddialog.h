// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QDialog>
#include <QStringList>

class QCheckBox;
class QLabel;
class QDialogButtonBox;
class QVBoxLayout;
class QComboBox;
class QLineEdit;

class ImgUploadDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImgUploadDialog(QDialog* parent = nullptr);

    // Per-upload sharing choice captured before the dialog closes. Empty /
    // absent when Google Drive is not the active backend (KTD7, R8, R10).
    QString selectedVisibility() const;
    QStringList recipients() const;

private slots:
    void onAccept();

private:
    static QStringList parseRecipients(const QString& text);

    QCheckBox* m_uploadWithoutConfirmation;
    QLabel* m_uploadLabel;
    QDialogButtonBox* buttonBox;
    QVBoxLayout* layout;

    // Drive-only controls (nullptr when Drive is not the active backend); the
    // dialog is shared infrastructure, so these are runtime-conditional rather
    // than compile-guarded.
    bool m_driveActive;
    QComboBox* m_visibility;
    QLabel* m_recipientsLabel;
    QLineEdit* m_recipients;
};
