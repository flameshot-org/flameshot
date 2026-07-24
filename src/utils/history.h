#pragma once

#define HISTORYPIXMAP_MAX_PREVIEW_WIDTH 250
#define HISTORYPIXMAP_MAX_PREVIEW_HEIGHT 100

#include <QList>
#include <QPixmap>
#include <QString>

struct HistoryFileName
{
    QString file;
    QString token;
    QString type;
};

class History
{
public:
    History();

    void save(const QPixmap&, const QString&);
    // Remove one cached entry, named the same packed way save() names it (the
    // shape packFileName produces and the uploaders retain). The single
    // removal path both delete controls use; tolerates an absent file.
    void remove(const QString& fileNamePacked);
    const QList<QString>& history();
    const QString& path();

    const HistoryFileName& unpackFileName(const QString&);
    const QString& packFileName(const QString&, const QString&, const QString&);

    // Drive file IDs contain '-', so they are stored hex-encoded in the packed
    // token slot (KTD8). These helpers own that encoding and the reconstructed
    // display URL, shared by the Drive uploader and the history view.
    static QString encodeDriveFileId(const QString& fileId);
    static QString decodeDriveFileId(const QString& token);
    static QString driveFileUrl(const QString& fileId);

private:
    QString m_historyPath;
    QList<QString> m_thumbs;

    // temporary variables
    QString m_packedFileName;
    HistoryFileName m_unpackedFileName;
};
