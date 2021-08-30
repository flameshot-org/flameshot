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

    const QList<QStringList>& captureShortcutsDefault(
      const QList<CaptureToolButton::ButtonType>& buttons);
    const QKeySequence& captureShortcutDefault(const QString& buttonType);

private:
    QList<QStringList> m_shortcuts;
    QKeySequence m_ks;

    void addShortcut(const QString& shortcutName, const QString& description);
};

#endif // CONFIGSHORTCUTS_H
