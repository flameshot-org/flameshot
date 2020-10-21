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
    const QKeySequence& captureShortcutDefault(
      const CaptureToolButton::ButtonType& buttonType);

private:
    QVector<QStringList> m_shortcuts;
    QKeySequence m_ks;
};

#endif // CONFIGSHORTCUTS_H
