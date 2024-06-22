#ifndef CAPTUREHISTORYUTILS_H
#define CAPTUREHISTORYUTILS_H
#include <qlist.h>
class QPixmap;
#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qrect.h>
#include <QtCore/qsharedpointer.h>

class BacktrackUtils : public QObject
{
    Q_OBJECT
public:
    /**
     * get a singleton object method
     * @return singleton object
     */
    static QSharedPointer<BacktrackUtils> getInstance();
    /**
     *
     * @return a screenshot
     */
    QSharedPointer<QPixmap> currentScreenShot();
    /**
     *
     * @return the selection in screenshot
     */
    QRect cureentSelection();

    /**
     * whether the history index is -1,
     * which means you want to exit the history mode
     * @return
     */
    [[nodiscard]] bool isNewest() const noexcept;

    /**
     * save the caption to cache diir
     * @param currentScreen current screenshot
     * @param selection current selection
     */
    void saveCapture(const QPixmap& currentScreen, const QRect& selection);
    void refreshValue();

    void fetchOlder();
    void fetchNewer();
    void fetchNewest();

private:
    BacktrackUtils();
    void deleteReduntantCache();
    void checkCache();
    QStringList m_fileList;
    int m_fileListIndex;
    inline QString getLastCacheName();

    QSharedPointer<QPixmap> m_currentScreenShot;
    QRect m_currentSelection;

    enum SelectionParseState : quint8
    {
        PS_BEGIN,
        PS_END,
        PS_WIDTH,
        PS_HEIGHT,
        PS_SEP_1,
        PS_SEP_2,
        PS_SEP_3,
        PS_LEFT,
        PS_TOP,
        PS_ERROR,
        PS_SUCCESS,
    } m_selParseState = PS_BEGIN;
};

#endif // CAPTUREHISTORYUTILS_H
