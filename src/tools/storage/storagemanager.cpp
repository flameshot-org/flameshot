#include "storagemanager.h"
#include "imguploader.h"
#include "imgur/imguruploadertool.h"
#include "s3/imgs3settings.h"
#include "s3/imgs3uploadertool.h"
#include "src/tools/capturetool.h"
#include <QSettings>
#include <QStringLiteral>

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
        ImgS3Settings s3Settings;
        m_qstr = s3Settings.url();
    } else if (imgUploaderType == SCREENSHOT_STORAGE_TYPE_IMGUR) {
        m_qstr = "https://i.imgur.com/";
    }
    return m_qstr;
}

const QString& StorageManager::storageDefault()
{
    ImgS3Settings imgS3Settings;
    if (!imgS3Settings.xApiKey().isEmpty()) {
        m_qstr = SCREENSHOT_STORAGE_TYPE_S3;
    } else {
        m_qstr = SCREENSHOT_STORAGE_TYPE_IMGUR;
    }
    return m_qstr;
}

bool StorageManager::storageLocked()
{
    // TODO - move this to some common config file, not a storage specific
    // configuration file
    bool res = false;
    ImgS3Settings imgS3Settings;
    if (imgS3Settings.settings()->contains("STORAGE_LOCK")) {
        res = imgS3Settings.settings()
                ->value(QStringLiteral("STORAGE_LOCK"))
                .toBool();
    }
    return res;
}