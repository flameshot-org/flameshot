#include "storagemanager.h"
#include "imguploader.h"
#include "imgur/imguruploader.h"
#include "imgur/imguruploadertool.h"
#include "s3/imgs3uploader.h"
#include "s3/imgs3uploadertool.h"
#include "src/tools/capturetool.h"
#include "src/tools/storage/s3/imgs3settings.h"

#include <QWidget>

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
