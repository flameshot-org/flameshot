#include "storagemanager.h"
#include "imguploader.h"
#include "imgur/imguruploadertool.h"
#include "s3/imgs3uploadertool.h"
#include "src/tools/capturetool.h"
#include <QSettings>

StorageManager::StorageManager() {}

CaptureTool* StorageManager::imgUploaderTool(const QString& imgUploaderType,
                                             QObject* parent)
{
    if (imgUploaderType == SCREENSHOT_STORAGE_TYPE_S3) {
        return new ImgS3UploaderTool(parent);
    } else if (imgUploaderType == SCREENSHOT_STORAGE_TYPE_IMGUR) {
        return new ImgurUploaderTool(parent);
    }
    return nullptr;
}

const QString& StorageManager::storageUrl(const QString& imgUploaderType)
{
    if (imgUploaderType == SCREENSHOT_STORAGE_TYPE_S3) {
        m_qstr = m_imgS3Settings.url();
    } else if (imgUploaderType == SCREENSHOT_STORAGE_TYPE_IMGUR) {
        m_qstr = "https://i.imgur.com/";
    }
    return m_qstr;
}

const QString& StorageManager::storageDefault()
{
    if (!m_imgS3Settings.storageLocked().isEmpty()) {
        m_qstr = m_imgS3Settings.storageLocked();
    } else {
        m_qstr = SCREENSHOT_STORAGE_TYPE_S3;
    }
    return m_qstr;
}

const QString& StorageManager::storageLocked()
{
    return m_imgS3Settings.storageLocked();
}