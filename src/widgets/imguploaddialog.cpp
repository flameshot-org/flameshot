// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "imguploaddialog.h"
#include "tools/imgupload/imguploadermanager.h"
#include "utils/confighandler.h"
#include "utils/globalvalues.h"
#include "widgets/recipientchipedit.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

// Only the Drive suggestion source is compile-guarded. This dialog is shared
// infrastructure -- it is built whenever either upload backend is enabled -- so
// a Drive type referenced unconditionally would break the Imgur-only build. The
// chip field itself is backend-neutral and needs no guard (KTD4).
#ifdef ENABLE_GDRIVE
#include "tools/imgupload/storages/gdrive/gdrivedirectory.h"
#endif

ImgUploadDialog::ImgUploadDialog(QDialog* parent)
  : QDialog(parent)
  // Ask the same resolver the uploader itself uses (ImgUploaderManager),
  // rather than re-deriving backend selection from the raw config value:
  // on a build with only one backend compiled in, that backend is always
  // the effective one regardless of what "uploadStorage" happens to hold.
  , m_driveActive(ImgUploaderManager().uploaderPlugin() ==
                  QStringLiteral("gdrive"))
  , m_visibility(nullptr)
  , m_recipientsLabel(nullptr)
  , m_recipients(nullptr)
{
    // No WA_DeleteOnClose: callers read the selection back after exec()
    // returns, so the dialog's lifetime belongs to whoever opened it.
    setMinimumSize(400, 120);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowTitle(tr("Upload Confirmation"));

    layout = new QVBoxLayout(this);

    m_uploadLabel = new QLabel(tr("Do you want to upload this capture?"), this);
    layout->addWidget(m_uploadLabel);

    // Per-upload visibility selector, only when Google Drive is active.
    if (m_driveActive) {
        m_visibility = new QComboBox(this);
        m_visibility->addItem(tr("Anyone in your organization with the link"),
                              QStringLiteral("domain"));
        m_visibility->addItem(tr("Private (only you)"),
                              QStringLiteral("private"));
        m_visibility->addItem(tr("Specific people by email"),
                              QStringLiteral("users"));
        m_visibility->addItem(tr("Anyone on the internet with the link"),
                              QStringLiteral("anyone"));
        const int defaultIndex =
          m_visibility->findData(ConfigHandler().gdriveDefaultVisibility());
        if (defaultIndex >= 0) {
            m_visibility->setCurrentIndex(defaultIndex);
        }
        layout->addWidget(m_visibility);

        m_recipientsLabel =
          new QLabel(tr("Recipients (name, email address, or group):"), this);
        m_recipients = new RecipientChipEdit(this);
#ifdef ENABLE_GDRIVE
        // The only Drive-specific line in this dialog. Without a source the
        // field is still a working recipient field, which is also what a user
        // who declined the directory scope gets (R11).
        m_recipients->setSuggestionSource(GDriveDirectory::instance());
#endif
        layout->addWidget(m_recipientsLabel);
        layout->addWidget(m_recipients);

        auto updateRecipientsVisibility = [this](int) {
            const bool users =
              m_visibility->currentData().toString() == QStringLiteral("users");
            m_recipientsLabel->setVisible(users);
            m_recipients->setVisible(users);
        };
        connect(m_visibility,
                static_cast<void (QComboBox::*)(int)>(
                  &QComboBox::currentIndexChanged),
                this,
                updateRecipientsVisibility);
        updateRecipientsVisibility(m_visibility->currentIndex());
    }

    buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);
    connect(
      buttonBox, &QDialogButtonBox::accepted, this, &ImgUploadDialog::onAccept);
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
            // Text typed but not yet committed counts as a recipient:
            // confirming the dialog must not wait on a lookup that may still be
            // in flight (R8). Anything left over failed validation, and saying
            // so beats dropping it -- an address discarded in silence is how a
            // typo used to reach the upload and fail afterwards.
            const QString leftover = m_recipients->commitPendingText();
            if (!leftover.isEmpty()) {
                m_uploadLabel->setText(tr("\"%1\" is not a valid email "
                                          "address. Correct or remove it.")
                                         .arg(leftover));
                return;
            }
            if (m_recipients->addresses().isEmpty()) {
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
        // Resolved primary addresses where a suggestion was picked, and the
        // text as typed everywhere else -- which is what the uploader's
        // per-recipient user-then-group retry still expects.
        return m_recipients->addresses();
    }
    return QStringList();
}
