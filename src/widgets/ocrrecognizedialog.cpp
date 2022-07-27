// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Cavivie & Contributors

#include "ocrrecognizedialog.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

OcrRecognizeDialog::OcrRecognizeDialog(QDialog* parent)
  : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(400, 120);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowTitle(tr("OCR Confirmation"));

    layout = new QVBoxLayout(this);

    m_ocrLabel = new QLabel(tr("Do you want to recognize this capture with OCR?"), this);

    layout->addWidget(m_ocrLabel);

    buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttonBox);

    m_ocrWithoutConfirmation =
      new QCheckBox(tr("Recognize without confirmation"), this);
    m_ocrWithoutConfirmation->setToolTip(tr("Recognize without confirmation"));
    connect(m_ocrWithoutConfirmation, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setOcrWithoutConfirmation(checked);
    });

    layout->addWidget(m_ocrWithoutConfirmation);
}
