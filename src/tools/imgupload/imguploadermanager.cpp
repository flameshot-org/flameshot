// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Yurii Puchkov & Contributors
//

#include "imguploadermanager.h"
#include <QPixmap>
#include <QWidget>

// TODO - remove this hard-code and create plugin manager in the future, you may
// include other storage headers here
#include "storages/catbox/catboxuploader.h"
#include "storages/imgur/imguruploader.h"

ImgUploaderManager::ImgUploaderManager(QObject* parent)
  : QObject(parent)
  , m_imgUploaderBase(nullptr)
{
    // TODO - implement ImgUploader for other Storages and selection among them
    m_imgUploaderPlugin = IMG_UPLOADER_STORAGE_DEFAULT;
    init();
}

void ImgUploaderManager::init()
{
    if (uploaderPlugin().compare("catbox") == 0) {
        m_qstrUrl = "https://catbox.moe/api.php";
        m_imgUploaderPlugin = "catbox";
    } else {
        m_qstrUrl = "https://imgur.com/";
        m_imgUploaderPlugin = "imgur";
    }
}

ImgUploaderBase* ImgUploaderManager::uploader(const QPixmap& capture,
                                              QWidget* parent)
{
    if (uploaderPlugin().compare("catbox") == 0) {
        m_imgUploaderBase =
          (ImgUploaderBase*)(new CatboxUploader(capture, parent));
    } else {
        m_imgUploaderBase =
          (ImgUploaderBase*)(new ImgurUploader(capture, parent));
    }

    if (m_imgUploaderBase && !capture.isNull()) {
        m_imgUploaderBase->upload();
    }
    return m_imgUploaderBase;
}

ImgUploaderBase* ImgUploaderManager::uploader(const QString& imgUploaderPlugin)
{
    m_imgUploaderPlugin = imgUploaderPlugin;
    init();
    return uploader(QPixmap());
}

const QString& ImgUploaderManager::uploaderPlugin()
{
    return m_imgUploaderPlugin;
}

const QString& ImgUploaderManager::url()
{
    return m_urlString;
}
