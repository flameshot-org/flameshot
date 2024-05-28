#include "capturehistoryutils.h"

#include "abstractlogger.h"

#include <QPaintEvent>
#include <QPainter>
#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include "cacheutils.h"
#include <QtCore/qchar.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qrect.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qglobal.h>
#include <QtCore/qfileinfo.h>

QSharedPointer<CaptureHistoryUtils> CaptureHistoryUtils::getInstance()
{
    static auto singleton =
      QSharedPointer<CaptureHistoryUtils>(new CaptureHistoryUtils);
    return singleton;
}

QSharedPointer<QPixmap> CaptureHistoryUtils::currentScreenShot()
{
    return m_currentScreenShot;
}

QRect CaptureHistoryUtils::cureentSelection()
{
    return m_currentSelection;
}
bool CaptureHistoryUtils::isNewest() const noexcept
{
    return m_fileListIndex == -1;
}

void CaptureHistoryUtils::saveCapture(const QPixmap& currentScreen,
                                      const QRect& selection)
{
    auto currentTime = QDateTime::currentDateTime();
    auto selectionSerialize = QString("+%1x%2+%3+%4+")
                                .arg(selection.width())
                                .arg(selection.height())
                                .arg(selection.x())
                                .arg(selection.y());
    auto cachePath = getCachePath() + "/cap.his." + selectionSerialize + "." +
                     currentTime.toString("yyyy-MM-dd hh:mm:ss") + ".png";
    QFile file(cachePath);

    file.open(QFile::Truncate | QFile::WriteOnly);
    currentScreen.save(&file, "PNG");

    m_fileList.prepend(cachePath);

    deleteReduntantCache();
}

void CaptureHistoryUtils::refreshValue()
{
    if (m_fileList.empty()) {
        m_fileListIndex = -1;
        return;
    }
    if (m_fileListIndex == -1) {
        return;
    }
    auto filename = m_fileList.at(m_fileListIndex);
    QPixmap pic;
    if (pic.load(filename, "PNG")) {
        m_currentScreenShot = QSharedPointer<QPixmap>::create(pic);
    }
    QFileInfo const info(filename);
    auto fileNameShort = info.fileName();

    const QString prefix = "cap.his.";
    auto selelctionStringIdxStart = fileNameShort.lastIndexOf(prefix);
    selelctionStringIdxStart += prefix.length();

    int width = 0;
    int height = 0;
    int xPos = 0;
    int yPos = 0;
    auto tempIdx = selelctionStringIdxStart;
    auto selParseState = PS_BEGIN;
    while (true) {
        if (selParseState == PS_ERROR || selParseState == PS_SUCCESS) {
            break;
        }
        switch (selParseState) {
            case PS_BEGIN:
                tempIdx++;
                selParseState = PS_WIDTH;
                break;
            case PS_WIDTH: {
                if (fileNameShort[tempIdx].isDigit()) {
                    width = width * 10 + (fileNameShort[tempIdx].digitValue());
                    tempIdx++;

                } else if (fileNameShort[tempIdx] == QChar(u'-') &&
                           (tempIdx + 1) < fileNameShort.length() &&
                           fileNameShort[tempIdx + 1].isDigit()) {
                    width = width * -1;
                    tempIdx++;
                } else {
                    selParseState = PS_SEP_1;
                }

                break;
            }
            case PS_SEP_1: {
                if (fileNameShort[tempIdx] != QChar(u'x')) {
                    selParseState = PS_ERROR;
                } else {
                    tempIdx++;
                    selParseState = PS_HEIGHT;
                }
                break;
            }
            case PS_HEIGHT: {
                if (fileNameShort[tempIdx].isDigit()) {
                    height = height * 10 + fileNameShort[tempIdx].digitValue();
                    tempIdx++;
                } else if (fileNameShort[tempIdx] == QChar(u'-') &&
                           (tempIdx + 1) < fileNameShort.length() &&
                           fileNameShort[tempIdx + 1].isDigit()) {
                    height = height * -1;
                    tempIdx++;
                } else {
                    selParseState = PS_SEP_2;
                }

            } break;
            case PS_SEP_2: {
                if (fileNameShort[tempIdx] != QChar(u'+')) {
                    selParseState = PS_ERROR;
                } else {
                    tempIdx++;
                    selParseState = PS_LEFT;
                }
                break;
            }
            case PS_LEFT: {
                if (fileNameShort[tempIdx].isDigit()) {
                    xPos = xPos * 10 + fileNameShort[tempIdx].digitValue();
                    tempIdx++;
                } else {
                    selParseState = PS_SEP_3;
                }

                break;
            }
            case PS_SEP_3: {
                if (fileNameShort[tempIdx] != QChar(u'+')) {
                    selParseState = PS_ERROR;
                    break;
                } else {
                    tempIdx++;
                    selParseState = PS_TOP;
                }
                break;
            }
            case PS_TOP: {
                if (fileNameShort[tempIdx].isDigit()) {
                    yPos = yPos * 10 + fileNameShort[tempIdx].digitValue();
                    tempIdx++;
                } else {
                    selParseState = PS_END;
                }

                break;
            }
            case PS_END: {
                if (fileNameShort[tempIdx] != QChar(u'+')) {
                    selParseState = PS_ERROR;
                    break;
                }
                selParseState = PS_SUCCESS;
                break;
            }
            default:
                break;
        }
    }
    if (selParseState == PS_ERROR) {
        AbstractLogger::error() << "cache pic selection parse errror";
        return;
    }
    m_currentSelection.setX(xPos);
    m_currentSelection.setY(yPos);
    m_currentSelection.setWidth(width);
    m_currentSelection.setHeight(height);
}

void CaptureHistoryUtils::fetchOlder()
{
    if (m_fileListIndex < m_fileList.length() - 1) {
        m_fileListIndex++;
        refreshValue();
    }
}

void CaptureHistoryUtils::fetchNewer()
{
    if (m_fileListIndex > 0) {
        m_fileListIndex--;
        refreshValue();
    }
    else if (m_fileListIndex == 0) {
        m_fileListIndex--;
    }
}

void CaptureHistoryUtils::fetchNewest()
{
    m_fileListIndex = 0;
    refreshValue();
}

CaptureHistoryUtils::CaptureHistoryUtils()
  : m_fileListIndex(-1)
{
    checkCache();
}

void CaptureHistoryUtils::deleteReduntantCache()
{
    quint32 maxHis = 100;
    if (m_fileList.length() >= maxHis) {
        auto fileDeleteNum = m_fileList.length() - maxHis;
        for (int i = 0; i < fileDeleteNum; i++) {
            QFile::remove(getLastCacheName());
        }
    }
}

void CaptureHistoryUtils::checkCache()
{

    QDir dir(getCachePath());
    QStringList nameFilters;
    nameFilters.append("cap.his.*.png");
    dir.setNameFilters(nameFilters);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Time);
    m_fileList = dir.entryList();
    std::transform(m_fileList.begin(),
                   m_fileList.end(),
                   m_fileList.begin(),
                   [](const auto& filename) {
                       return getCachePath() + QDir::separator() + filename;
                   });

    refreshValue();
}

QString CaptureHistoryUtils::getLastCacheName()
{
    return m_fileList[m_fileList.length() - 1];
}
