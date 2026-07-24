// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "imguploaddialog.h"
#include "utils/confighandler.h"
#include "utils/globalvalues.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QVBoxLayout>

ImgUploadDialog::ImgUploadDialog(QDialog* parent)
  : QDialog(parent)
  , m_driveActive(ConfigHandler().uploadStorage() == QStringLiteral("gdrive"))
  , m_visibility(nullptr)
  , m_recipientsLabel(nullptr)
  , m_recipients(nullptr)
{
    // No WA_DeleteOnClose: callers read the selection back after exec() returns,
    // so the dialog's lifetime belongs to whoever opened it.
    setMinimumSize(400, 120);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowTitle(tr("Upload Confirmation"));

    layout = new QVBoxLayout(this);

    m_uploadLabel = new QLabel(tr("Do you want to upload this capture?"), this);
    layout->addWidget(m_uploadLabel);

    // Per-upload visibility selector, only when Google Drive is active.
    if (m_driveActive) {
        m_visibility = new QComboBox(this);
        m_visibility->addItem(
          tr("Anyone in your organization with the link"),
          QStringLiteral("domain"));
        m_visibility->addItem(tr("Private (only you)"),
                              QStringLiteral("private"));
        m_visibility->addItem(tr("Specific people by email"),
                              QStringLiteral("users"));
        m_visibility->addItem(tr("Anyone on the internet with the link"),
                              QStringLiteral("anyone"));
        const int defaultIndex = m_visibility->findData(
          ConfigHandler().gdriveDefaultVisibility());
        if (defaultIndex >= 0) {
            m_visibility->setCurrentIndex(defaultIndex);
        }
        layout->addWidget(m_visibility);

        m_recipientsLabel =
          new QLabel(tr("Recipient emails (comma-separated):"), this);
        m_recipients = new QLineEdit(this);
        layout->addWidget(m_recipientsLabel);
        layout->addWidget(m_recipients);

        auto updateRecipientsVisibility = [this](int) {
            const bool users = m_visibility->currentData().toString() ==
                               QStringLiteral("users");
            m_recipientsLabel->setVisible(users);
            m_recipients->setVisible(users);
        };
        connect(
          m_visibility,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this,
          updateRecipientsVisibility);
        updateRecipientsVisibility(m_visibility->currentIndex());
    }

    buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);
    connect(buttonBox, &QDialogButtonBox::accepted, this,
            &ImgUploadDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    m_uploadWithoutConfirmation =
      new QCheckBox(tr("Upload without confirmation"), this);
    m_uploadWithoutConfirmation->setToolTip(tr("Upload without confirmation"));
    connect(m_uploadWithoutConfirmation, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setUploadWithoutConfirmation(checked);
    });
    layout->addWidget(m_uploadWithoutConfirmation);
}

void ImgUploadDialog::onAccept()
{
    if (m_driveActive && m_visibility) {
        const QString level = m_visibility->currentData().toString();
        if (level == QStringLiteral("users")) {
            const QStringList list = parseRecipients(m_recipients->text());
            if (list.isEmpty()) {
                m_uploadLabel->setText(
                  tr("Enter at least one valid recipient email address."));
                return;
            }
        }
        if (level == QStringLiteral("anyone")) {
            // "Public" gets a one-step, unambiguous confirmation.
            if (QMessageBox::warning(
                  this,
                  tr("Share publicly?"),
                  tr("\"Anyone on the internet with the link\" makes this "
                     "screenshot readable by anyone who has the link. "
                     "Continue?"),
                  QMessageBox::Yes | QMessageBox::No,
                  QMessageBox::No) != QMessageBox::Yes) {
                return;
            }
        }
    }
    accept();
}

QStringList ImgUploadDialog::parseRecipients(const QString& text)
{
    static const QRegularExpression separator(QStringLiteral("[,;\\s]+"));
    static const QRegularExpression emailPattern(
      QStringLiteral("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$"));

    QStringList valid;
    const QStringList parts =
      text.split(separator, Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        const QString candidate = part.trimmed();
        if (!candidate.isEmpty() &&
            emailPattern.match(candidate).hasMatch()) {
            valid.append(candidate);
        }
    }
    return valid;
}

QString ImgUploadDialog::selectedVisibility() const
{
    if (m_driveActive && m_visibility) {
        return m_visibility->currentData().toString();
    }
    return QString();
}

QStringList ImgUploadDialog::recipients() const
{
    if (m_driveActive && m_recipients &&
        selectedVisibility() == QStringLiteral("users")) {
        return parseRecipients(m_recipients->text());
    }
    return QStringList();
}
