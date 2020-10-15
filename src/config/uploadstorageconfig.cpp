// Copyright(c) 2020 Yurii Puchkov at Namecheap & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "uploadstorageconfig.h"
#include "src/tools/storage/imgstorages.h"
#include "src/tools/storage/storagemanager.h"
#include "src/utils/confighandler.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

UploadStorageConfig::UploadStorageConfig(QWidget* parent)
  : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);

    QGroupBox* groupBox = new QGroupBox(tr("Upload storage"));

    // TODO - remove dependency injection (s3 & imgur)
    // imgur
    QRadioButton* storageImgUr = new QRadioButton(tr("Imgur storage"));
    connect(storageImgUr, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setUploadStorage(SCREENSHOT_STORAGE_TYPE_IMGUR);
    });

    // s3
    QRadioButton* storageImgS3 = new QRadioButton(
      tr("S3 storage (require config.ini file with s3 credentials)"));
    connect(storageImgS3, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setUploadStorage(SCREENSHOT_STORAGE_TYPE_S3);
    });

    StorageManager storageManager;
    if (storageManager.storageLocked()) {
        ConfigHandler().setUploadStorage(storageManager.storageDefault());
        storageImgUr->setDisabled(true);
        storageImgS3->setDisabled(true);
    }

    // set current storage radiobutton active
    if (ConfigHandler().uploadStorage() == SCREENSHOT_STORAGE_TYPE_IMGUR) {
        storageImgUr->setChecked(true);

    } else {
        storageImgS3->setChecked(true);
    }

    // draw configuration options for uploadStorage
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(storageImgUr);
    vbox->addWidget(storageImgS3);
    vbox->addStretch(1);
    groupBox->setLayout(vbox);
    m_layout->addWidget(groupBox);
}