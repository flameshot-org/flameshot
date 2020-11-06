#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include "s3/imgs3settings.h"
#include <QPixmap>
#include <QString>

class QObject;
class ImgUploader;
class CaptureTool;

class StorageManager
{
public:
    explicit StorageManager();

    CaptureTool* imgUploaderTool(const QString& imgUploaderType,
                                 QObject* parent = nullptr);
    const QString& storageUrl(const QString& imgUploaderType);
    const QString& storageDefault();
    const QString& storageLocked();

private:
    // class members
    QString m_qstr;
    ImgS3Settings m_imgS3Settings;
};

#endif // STORAGEMANAGER_H
