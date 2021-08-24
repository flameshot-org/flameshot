#ifndef CONFIGSHORTCUTS_H
#define CONFIGSHORTCUTS_H

#include "src/widgets/capture/capturetoolbutton.h"
#include <QKeySequence>
#include <QString>
#include <QStringList>
#include <QVector>

class ConfigShortcuts
{
public:
    ConfigShortcuts();

    const QVector<QStringList>& captureShortcutsDefault(
      const QVector<CaptureToolButton::ButtonType>& buttons);
    const QKeySequence& captureShortcutDefault(const QString& buttonType);

private:
    QVector<QStringList> m_shortcuts;
    QKeySequence m_ks;

    void addShortcut(const QString& shortcutName, const QString& description);
};

#endif // CONFIGSHORTCUTS_H
