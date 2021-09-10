#include "configshortcuts.h"
#include "confighandler.h"
#include "src/tools/capturetool.h"
#include <QMetaEnum>
#include <QVariant>

ConfigShortcuts::ConfigShortcuts() {}

const QList<QStringList>& ConfigShortcuts::captureShortcutsDefault(
  const QList<CaptureToolButton::ButtonType>& buttons)
{
    // get shortcuts names from capture buttons
    for (const CaptureToolButton::ButtonType& t : buttons) {
        CaptureToolButton* b = new CaptureToolButton(t, nullptr);
        QString shortcutName = QVariant::fromValue(t).toString();
        if (shortcutName != "TYPE_IMAGEUPLOADER") {
            addShortcut(shortcutName, b->tool()->description());
        }
        delete b;
    }

    // additional tools that don't have their own buttons
    addShortcut("TYPE_TOGGLE_PANEL", "Toggle side panel");
    addShortcut("TYPE_RESIZE_LEFT", "Resize selection left 1px");
    addShortcut("TYPE_RESIZE_RIGHT", "Resize selection right 1px");
    addShortcut("TYPE_RESIZE_UP", "Resize selection up 1px");
    addShortcut("TYPE_RESIZE_DOWN", "Resize selection down 1px");
    addShortcut("TYPE_SELECT_ALL", "Select entire screen");
    addShortcut("TYPE_MOVE_LEFT", "Move selection left 1px");
    addShortcut("TYPE_MOVE_RIGHT", "Move selection right 1px");
    addShortcut("TYPE_MOVE_UP", "Move selection up 1px");
    addShortcut("TYPE_MOVE_DOWN", "Move selection down 1px");
    addShortcut("TYPE_COMMIT_CURRENT_TOOL", "Commit text in text area");
    addShortcut("TYPE_DELETE_CURRENT_TOOL", "Delete current tool");

    // non-editable shortcuts have an empty shortcut name

    m_shortcuts << (QStringList() << "" << QObject::tr("Quit capture")
                                  << QKeySequence(Qt::Key_Escape).toString());

    // Global hotkeys
#if defined(Q_OS_MACOS)
    m_shortcuts << (QStringList()
                    << "" << QObject::tr("Screenshot history") << "⇧⌘⌥H");
    m_shortcuts << (QStringList()
                    << "" << QObject::tr("Capture screen") << "⇧⌘⌥4");
#elif defined(Q_OS_WIN)
    m_shortcuts << (QStringList() << "" << QObject::tr("Screenshot history")
                                  << "Shift+Print Screen");
    m_shortcuts << (QStringList()
                    << "" << QObject::tr("Capture screen") << "Print Screen");
#else
    // TODO - Linux doesn't support global shortcuts for (XServer and Wayland),
    // possibly it will be solved in the QHotKey library later. So it is
    // disabled for now.
#endif
    m_shortcuts << (QStringList()
                    << "" << QObject::tr("Show color picker") << "Right Click");
    m_shortcuts << (QStringList()
                    << "" << QObject::tr("Change the tool's thickness")
                    << "Mouse Wheel");

    return m_shortcuts;
}

// Helper function
void ConfigShortcuts::addShortcut(const QString& shortcutName,
                                  const QString& description)
{
    m_shortcuts << (QStringList()
                    << shortcutName
                    << QObject::tr(description.toStdString().c_str())
                    << ConfigHandler().shortcut(shortcutName));
}
