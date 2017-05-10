#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QPixmap>
#include <QRect>

class QString;
class CaptureModification;

class Screenshot
{
public:
    Screenshot(const QPixmap &);

    void setScreenshot(const QPixmap &);
    QPixmap getScreenshot() const;
    QString graphicalSave(const QRect &selection = QRect()) const;
    QPixmap paintModifications(const QVector<CaptureModification>);

private:
    QPixmap m_screenshot;
};

#endif // SCREENSHOT_H
