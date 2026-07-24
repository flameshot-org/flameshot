// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Yurii Puchkov & Contributors

#pragma once

#include "tools/imgupload/storages/imguploaderbase.h"

#include <QObject>
#include <QStringList>

#define IMG_UPLOADER_STORAGE_DEFAULT "imgur"

class QPixmap;
class QWidget;

class ImgUploaderManager : public QObject
{
    Q_OBJECT
public:
    explicit ImgUploaderManager(QObject* parent = nullptr);

    ImgUploaderBase* uploader(const QPixmap& capture,
                              QWidget* parent = nullptr);
    // Upload carrying a per-upload Drive sharing selection (ignored by
    // non-Drive backends).
    ImgUploaderBase* uploader(const QPixmap& capture,
                              QWidget* parent,
                              const QString& visibility,
                              const QStringList& recipients);
    ImgUploaderBase* uploader(const QString& imgUploaderPlugin);

    const QString& url();
    const QString& uploaderPlugin();

private:
    void init();
    ImgUploaderBase* createUploader(const QPixmap& capture, QWidget* parent);

private:
    ImgUploaderBase* m_imgUploaderBase;
    QString m_urlString;
    QString m_imgUploaderPlugin;
};
