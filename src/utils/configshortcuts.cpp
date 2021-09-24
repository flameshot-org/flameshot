#include "configshortcuts.h"
#include "src/tools/capturetool.h"
#include <QMetaEnum>
#include <QVariant>

ConfigShortcuts::ConfigShortcuts() {}

const QVector<QStringList>& ConfigShortcuts::captureShortcutsDefault(
  const QVector<CaptureTool::Type>& buttons)
{
    // get shortcuts names from capture buttons
    for (const CaptureTool::Type& t : buttons) {
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

const QKeySequence& ConfigShortcuts::captureShortcutDefault(
  const QString& buttonType)
{
    m_ks = QKeySequence();
    if (buttonType == "TYPE_PENCIL") {
        m_ks = QKeySequence(Qt::Key_P);
    } else if (buttonType == "TYPE_DRAWER") {
        m_ks = QKeySequence(Qt::Key_D);
    } else if (buttonType == "TYPE_ARROW") {
        m_ks = QKeySequence(Qt::Key_A);
    } else if (buttonType == "TYPE_SELECTION") {
        m_ks = QKeySequence(Qt::Key_S);
    } else if (buttonType == "TYPE_RECTANGLE") {
        m_ks = QKeySequence(Qt::Key_R);
    } else if (buttonType == "TYPE_CIRCLE") {
        m_ks = QKeySequence(Qt::Key_C);
    } else if (buttonType == "TYPE_MARKER") {
        m_ks = QKeySequence(Qt::Key_M);
    } else if (buttonType == "TYPE_MOVESELECTION") {
        m_ks = QKeySequence(Qt::CTRL + Qt::Key_M);
    } else if (buttonType == "TYPE_UNDO") {
        m_ks = QKeySequence(Qt::CTRL + Qt::Key_Z);
    } else if (buttonType == "TYPE_COPY") {
        m_ks = QKeySequence(Qt::CTRL + Qt::Key_C);
    } else if (buttonType == "TYPE_SAVE") {
        m_ks = QKeySequence(Qt::CTRL + Qt::Key_S);
    } else if (buttonType == "TYPE_EXIT") {
        m_ks = QKeySequence(Qt::CTRL + Qt::Key_Q);
    } else if (buttonType == "TYPE_IMAGEUPLOADER") {
        m_ks = QKeySequence(Qt::CTRL + Qt::Key_U);
    }
#if !defined(Q_OS_MACOS)
    else if (buttonType == "TYPE_OPEN_APP") {
        m_ks = QKeySequence(Qt::CTRL + Qt::Key_O);
    }
#endif
    else if (buttonType == "TYPE_PIXELATE") {
        m_ks = QKeySequence(Qt::Key_B);
    } else if (buttonType == "TYPE_REDO") {
        m_ks = QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
    } else if (buttonType == "TYPE_TEXT") {
        m_ks = QKeySequence(Qt::Key_T);
    } else if (buttonType == "TYPE_INVERT") {
        m_ks = QKeySequence(Qt::Key_I);
    } else if (buttonType == "TYPE_TOGGLE_PANEL") {
        m_ks = QKeySequence(Qt::Key_Space);
    } else if (buttonType == "TYPE_RESIZE_LEFT") {
        m_ks = QKeySequence(Qt::SHIFT + Qt::Key_Left);
    } else if (buttonType == "TYPE_RESIZE_RIGHT") {
        m_ks = QKeySequence(Qt::SHIFT + Qt::Key_Right);
    } else if (buttonType == "TYPE_RESIZE_UP") {
        m_ks = QKeySequence(Qt::SHIFT + Qt::Key_Up);
    } else if (buttonType == "TYPE_RESIZE_DOWN") {
        m_ks = QKeySequence(Qt::SHIFT + Qt::Key_Down);
    } else if (buttonType == "TYPE_SELECT_ALL") {
        m_ks = QKeySequence(Qt::CTRL + Qt::Key_A);
    } else if (buttonType == "TYPE_MOVE_LEFT") {
        m_ks = QKeySequence(Qt::Key_Left);
    } else if (buttonType == "TYPE_MOVE_RIGHT") {
        m_ks = QKeySequence(Qt::Key_Right);
    } else if (buttonType == "TYPE_MOVE_UP") {
        m_ks = QKeySequence(Qt::Key_Up);
    } else if (buttonType == "TYPE_MOVE_DOWN") {
        m_ks = QKeySequence(Qt::Key_Down);
    } else if (buttonType == "TYPE_COMMIT_CURRENT_TOOL") {
        m_ks = QKeySequence(Qt::CTRL + Qt::Key_Return);
    } else if (buttonType == "TYPE_DELETE_CURRENT_TOOL") {
#if defined(Q_OS_MACOS)
        m_ks = QKeySequence(Qt::Key_Backspace);
#else
        m_ks = QKeySequence(Qt::Key_Delete);
#endif
    }
    return m_ks;
}

// Helper function
void ConfigShortcuts::addShortcut(const QString& shortcutName,
                                  const QString& description)
{
    m_shortcuts << (QStringList()
                    << shortcutName
                    << QObject::tr(description.toStdString().c_str())
                    << captureShortcutDefault(shortcutName).toString());
}
