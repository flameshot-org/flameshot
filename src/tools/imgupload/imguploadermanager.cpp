// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Yurii Puchkov & Contributors
//

#include "imguploadermanager.h"
#include "utils/confighandler.h"
#ifdef ENABLE_IMGUR
#include "tools/imgupload/storages/imgur/imguruploader.h"
#endif
#ifdef ENABLE_GDRIVE
#include "tools/imgupload/storages/gdrive/gdriveuploader.h"
#endif

#include <QPixmap>
#include <QWidget>

ImgUploaderManager::ImgUploaderManager(QObject* parent)
  : QObject(parent)
  , m_imgUploaderBase(nullptr)
{
    // Start from the configured backend; init() normalizes it against the
    // backends actually compiled into this build.
    m_imgUploaderPlugin = ConfigHandler().uploadStorage();
    init();
}

void ImgUploaderManager::init()
{
    // Resolve m_imgUploaderPlugin to a compiled-in backend. This must honor an
    // explicitly requested plugin (the history-delete router sets it via the
    // uploader(QString) overload) instead of unconditionally forcing "imgur",
    // otherwise a Drive delete token would be handed to the Imgur backend.
#ifdef ENABLE_GDRIVE
    if (m_imgUploaderPlugin == QStringLiteral("gdrive")) {
        m_urlString = QStringLiteral("https://drive.google.com/");
        return;
    }
#endif
#ifdef ENABLE_IMGUR
    // Imgur is the default and the fallback for empty/unknown backend tags
    // (legacy one-part entries have an empty type; two-part entries carry a
    // type with an empty token).
    m_imgUploaderPlugin = QStringLiteral("imgur");
    m_urlString = QStringLiteral("https://imgur.com/");
    return;
#endif
#ifdef ENABLE_GDRIVE
    // Imgur is not compiled in: Google Drive is the only available backend.
    m_imgUploaderPlugin = QStringLiteral("gdrive");
    m_urlString = QStringLiteral("https://drive.google.com/");
#endif
}

ImgUploaderBase* ImgUploaderManager::createUploader(const QPixmap& capture,
                                                    QWidget* parent)
{
    ImgUploaderBase* uploader = nullptr;
#ifdef ENABLE_GDRIVE
    if (m_imgUploaderPlugin == QStringLiteral("gdrive")) {
        uploader = (ImgUploaderBase*)(new GDriveUploader(capture, parent));
    }
#endif
#ifdef ENABLE_IMGUR
    if (uploader == nullptr) {
        uploader = (ImgUploaderBase*)(new ImgurUploader(capture, parent));
    }
#endif
    return uploader;
}

ImgUploaderBase* ImgUploaderManager::uploader(const QPixmap& capture,
                                              QWidget* parent)
{
    m_imgUploaderBase = createUploader(capture, parent);
    if (m_imgUploaderBase && !capture.isNull()) {
        m_imgUploaderBase->upload();
    }
    return m_imgUploaderBase;
}

ImgUploaderBase* ImgUploaderManager::uploader(const QPixmap& capture,
                                              QWidget* parent,
                                              const QString& visibility,
                                              const QStringList& recipients)
{
    m_imgUploaderBase = createUploader(capture, parent);
#ifdef ENABLE_GDRIVE
    if (auto* drive = qobject_cast<GDriveUploader*>(m_imgUploaderBase)) {
        if (!visibility.isEmpty()) {
            drive->setVisibility(visibility);
        }
        drive->setRecipients(recipients);
    }
#else
    Q_UNUSED(visibility)
    Q_UNUSED(recipients)
#endif
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
