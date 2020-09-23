#include "configshortcuts.h"
#include "src/tools/capturetool.h"
#include <QDebug>

ConfigShortcuts::ConfigShortcuts() {}

// QVector<CaptureButton::ButtonType> getButtons()

const QVector<QStringList>& ConfigShortcuts::captureShortcutsDefault(
  const QVector<CaptureButton::ButtonType>& buttons)
{
    // get shortcuts names from tools
    for (const CaptureButton::ButtonType& t : buttons) {
        CaptureButton* b = new CaptureButton(t, nullptr);
        QString shortcutName = QVariant::fromValue(t).toString();
        QKeySequence ks = captureShortcutDefault(t);
        m_shortcuts << (QStringList()
                        << shortcutName << b->tool()->description()
                        << ks.toString());
        b->close();
    }

    m_shortcuts << (QStringList()
                    << "TYPE_TOGGLE_PANEL" << QObject::tr("Toggle side panel")
                    << QKeySequence(Qt::Key_Space).toString());

    m_shortcuts << (QStringList()
                    << "TYPE_RESIZE_LEFT"
                    << QObject::tr("Resize selection left 1px")
                    << QKeySequence(Qt::SHIFT + Qt::Key_Left).toString());
    m_shortcuts << (QStringList()
                    << "TYPE_RESIZE_RIGHT"
                    << QObject::tr("Resize selection right 1px")
                    << QKeySequence(Qt::SHIFT + Qt::Key_Right).toString());
    m_shortcuts << (QStringList()
                    << "TYPE_RESIZE_UP"
                    << QObject::tr("Resize selection up 1px")
                    << QKeySequence(Qt::SHIFT + Qt::Key_Up).toString());
    m_shortcuts << (QStringList()
                    << "TYPE_RESIZE_DOWN"
                    << QObject::tr("Resize selection down 1px")
                    << QKeySequence(Qt::SHIFT + Qt::Key_Down).toString());

    m_shortcuts << (QStringList() << "TYPE_MOVE_LEFT"
                                  << QObject::tr("Move selection left 1px")
                                  << QKeySequence(Qt::Key_Left).toString());
    m_shortcuts << (QStringList() << "TYPE_MOVE_RIGHT"
                                  << QObject::tr("Move selection right 1px")
                                  << QKeySequence(Qt::Key_Right).toString());
    m_shortcuts << (QStringList()
                    << "TYPE_MOVE_UP" << QObject::tr("Move selection up 1px")
                    << QKeySequence(Qt::Key_Up).toString());
    m_shortcuts << (QStringList() << "TYPE_MOVE_DOWN"
                                  << QObject::tr("Move selection down 1px")
                                  << QKeySequence(Qt::Key_Down).toString());

    m_shortcuts << (QStringList() << "" << QObject::tr("Quit capture")
                                  << QKeySequence(Qt::Key_Escape).toString());
    m_shortcuts << (QStringList() << "" << QObject::tr("Screenshot history")
                                  << "Shift+Print Screen");
    m_shortcuts << (QStringList()
                    << "" << QObject::tr("Capture screen") << "Print Screen");
    m_shortcuts << (QStringList()
                    << "" << QObject::tr("Show color picker") << "Right Click");
    m_shortcuts << (QStringList()
                    << "" << QObject::tr("Change the tool's thickness")
                    << "Mouse Wheel");

    return m_shortcuts;
}

const QKeySequence& ConfigShortcuts::captureShortcutDefault(
  const CaptureButton::ButtonType& buttonType)
{
    m_ks = QKeySequence();
    switch (buttonType) {
        case CaptureButton::ButtonType::TYPE_PENCIL:
            m_ks = QKeySequence(Qt::Key_P);
            break;
        case CaptureButton::ButtonType::TYPE_DRAWER:
            m_ks = QKeySequence(Qt::Key_D);
            break;
        case CaptureButton::ButtonType::TYPE_ARROW:
            m_ks = QKeySequence(Qt::Key_A);
            break;
        case CaptureButton::ButtonType::TYPE_SELECTION:
            m_ks = QKeySequence(Qt::Key_S);
            break;
        case CaptureButton::ButtonType::TYPE_RECTANGLE:
            m_ks = QKeySequence(Qt::Key_R);
            break;
        case CaptureButton::ButtonType::TYPE_CIRCLE:
            m_ks = QKeySequence(Qt::Key_C);
            break;
        case CaptureButton::ButtonType::TYPE_MARKER:
            m_ks = QKeySequence(Qt::Key_M);
            break;
            //    case CaptureButton::ButtonType::TYPE_SELECTIONINDICATOR:
        case CaptureButton::ButtonType::TYPE_MOVESELECTION:
            m_ks = QKeySequence(Qt::CTRL + Qt::Key_M);
            break;
        case CaptureButton::ButtonType::TYPE_UNDO:
            m_ks = QKeySequence(Qt::CTRL + Qt::Key_Z);
            break;
        case CaptureButton::ButtonType::TYPE_COPY:
            m_ks = QKeySequence(Qt::CTRL + Qt::Key_C);
            break;
        case CaptureButton::ButtonType::TYPE_SAVE:
            m_ks = QKeySequence(Qt::CTRL + Qt::Key_S);
            break;
        case CaptureButton::ButtonType::TYPE_EXIT:
            m_ks = QKeySequence(Qt::CTRL + Qt::Key_Q);
            break;
        case CaptureButton::ButtonType::TYPE_IMAGEUPLOADER:
            m_ks = QKeySequence(Qt::Key_Return);
            break;
        case CaptureButton::ButtonType::TYPE_OPEN_APP:
            m_ks = QKeySequence(Qt::CTRL + Qt::Key_O);
            break;
        case CaptureButton::ButtonType::TYPE_BLUR:
            m_ks = QKeySequence(Qt::Key_B);
            break;
        case CaptureButton::ButtonType::TYPE_REDO:
            m_ks = QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
            break;
            //    case CaptureButton::ButtonType::TYPE_PIN:
        case CaptureButton::ButtonType::TYPE_TEXT:
            m_ks = QKeySequence(Qt::Key_T);
            break;
        default:
            break;
    }
    return m_ks;
}
