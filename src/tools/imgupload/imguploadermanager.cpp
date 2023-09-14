// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Yurii Puchkov & Contributors
//

#include "imguploadermanager.h"
#include "confighandler.h"
#include "generalconf.h"
#include "storages/catbox/catboxuploader.h"
#include "storages/imgur/imguruploader.h"
#include <QPixmap>
#include <QWidget>
#include <cstdio>

ImgUploaderManager::ImgUploaderManager(QObject* parent)
  : QObject(parent)
  , m_imgUploaderBase(nullptr)
{
    m_imgUploaderPlugin = ConfigHandler().imgUploaderPlugin();
    if (m_imgUploaderPlugin.isEmpty())
        m_imgUploaderPlugin = IMG_UPLOADER_STORAGE_DEFAULT;
    init();
}

void ImgUploaderManager::init()
{
    if (uploaderPlugin().compare("catbox") == 0) {
        m_urlString = "https://files.catbox.moe/";
        m_imgUploaderPlugin = "catbox";
    } else {
        m_urlString = "https://imgur.com/";
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
