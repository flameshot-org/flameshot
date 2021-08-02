// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "imguploaddialog.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

ImgUploadDialog::ImgUploadDialog(QDialog* parent)
  : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(400, 120);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowTitle(tr("Upload Confirmation"));

    layout = new QVBoxLayout(this);

    m_uploadLabel = new QLabel(tr("Do you want to upload this capture?"), this);

    layout->addWidget(m_uploadLabel);

    buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
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