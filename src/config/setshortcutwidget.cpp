// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2020 Yurii Puchkov at Namecheap & Contributors

#include "setshortcutwidget.h"
#include "utils/globalvalues.h"

#include <QIcon>
#include <QKeyCombination>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QTimer>
#if defined(Q_OS_WIN)
#include <qt_windows.h>
#endif

SetShortcutDialog::SetShortcutDialog(QDialog* parent)
  : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowTitle(tr("Set Shortcut"));
    m_ks = QKeySequence();

    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignHCenter);

    auto* infoTop = new QLabel(tr("Enter new shortcut to change "));
    infoTop->setMargin(10);
    infoTop->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(infoTop);

    auto* infoIcon = new QLabel();
    infoIcon->setAlignment(Qt::AlignCenter);
    infoIcon->setPixmap(QPixmap(":/img/app/keyboard.svg"));
    m_layout->addWidget(infoIcon);

    m_layout->addWidget(infoIcon);

    QString msg = "";
#if defined(Q_OS_MACOS)
    msg = tr(
      "Press Esc to cancel or ⌘+Backspace to disable the keyboard shortcut.");
#else
    msg =
      tr("Press Esc to cancel or Backspace to disable the keyboard shortcut.");
#endif

    auto* infoBottom = new QLabel(msg);
    infoBottom->setMargin(10);
    infoBottom->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(infoBottom);

    // 0ms Delay: Event loop waits until after show(); widget fully initialized
    QTimer::singleShot(0, this, &SetShortcutDialog::startCapture);
}

SetShortcutDialog::~SetShortcutDialog()
{
    stopCapture();
}

void SetShortcutDialog::startCapture()
{
    grabKeyboard(); // Call AFTER show()!
    setFocus();
#if defined(Q_OS_WIN)
    // Print Screen can be delivered as WM_HOTKEY or only as a key-release
    // event on Windows. Register it temporarily while this dialog is active
    // so it can be selected even after the capture shortcut was changed away
    // from PrtSc. If registration fails, the key/native event fallbacks below
    // can still capture it.
    constexpr int printScreenCaptureHotkeyId = 0x4653; // "FS"
    m_printScreenHotkeyRegistered =
      RegisterHotKey(reinterpret_cast<HWND>(winId()),
                     printScreenCaptureHotkeyId,
                     0,
                     VK_SNAPSHOT);
#endif
}

void SetShortcutDialog::stopCapture()
{
    if (QWidget::keyboardGrabber() == this) {
        releaseKeyboard();
    }
#if defined(Q_OS_WIN)
    if (m_printScreenHotkeyRegistered) {
        constexpr int printScreenCaptureHotkeyId = 0x4653;
        UnregisterHotKey(reinterpret_cast<HWND>(winId()),
                         printScreenCaptureHotkeyId);
        m_printScreenHotkeyRegistered = false;
    }
#endif
}

const QKeySequence& SetShortcutDialog::shortcut()
{
    return m_ks;
}

void SetShortcutDialog::updateShortcutFromKeyEvent(QKeyEvent* event)
{
    int key = event->key();
#if defined(Q_OS_WIN)
    // Qt can report Print Screen only through nativeVirtualKey(), especially
    // when Windows emits no translated key-press event for VK_SNAPSHOT.
    if (event->nativeVirtualKey() == VK_SNAPSHOT) {
        key = Qt::Key_Print;
    }
#endif

    if (key == 0 || key == Qt::Key_unknown || key == Qt::Key_Shift ||
        key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Meta) {
        return;
    }

    const Qt::KeyboardModifiers modifiers =
      event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier |
                            Qt::AltModifier | Qt::MetaModifier);
    m_ks = QKeySequence(QKeyCombination(modifiers, static_cast<Qt::Key>(key)));
}

void SetShortcutDialog::keyPressEvent(QKeyEvent* event)
{
    updateShortcutFromKeyEvent(event);
    event->accept();
}

void SetShortcutDialog::keyReleaseEvent(QKeyEvent* event)
{
    // Print Screen commonly arrives as a release-only event on Windows.
    if (m_ks.isEmpty()) {
        updateShortcutFromKeyEvent(event);
    }

    if (m_ks == QKeySequence(Qt::Key_Escape)) {
        reject();
        return;
    }

    if (!m_ks.isEmpty()) {
        accept();
    }
}

#if defined(Q_OS_WIN)
bool SetShortcutDialog::nativeEvent(const QByteArray& eventType,
                                    void* message,
                                    qintptr* result)
{
    Q_UNUSED(eventType)

    auto* msg = static_cast<MSG*>(message);
    if (msg == nullptr) {
        return QDialog::nativeEvent(eventType, message, result);
    }

    constexpr int printScreenCaptureHotkeyId = 0x4653;
    if (msg->message == WM_HOTKEY &&
        static_cast<int>(msg->wParam) == printScreenCaptureHotkeyId) {
        m_ks = QKeySequence(Qt::Key_Print);
        if (result != nullptr) {
            *result = 0;
        }
        QTimer::singleShot(0, this, &SetShortcutDialog::accept);
        return true;
    }

    const bool isKeyMessage =
      msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN ||
      msg->message == WM_KEYUP || msg->message == WM_SYSKEYUP;
    if (isKeyMessage && msg->wParam == VK_SNAPSHOT) {
        Qt::KeyboardModifiers modifiers = Qt::NoModifier;
        if (GetKeyState(VK_SHIFT) & 0x8000) {
            modifiers |= Qt::ShiftModifier;
        }
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            modifiers |= Qt::ControlModifier;
        }
        if (GetKeyState(VK_MENU) & 0x8000) {
            modifiers |= Qt::AltModifier;
        }
        if ((GetKeyState(VK_LWIN) & 0x8000) ||
            (GetKeyState(VK_RWIN) & 0x8000)) {
            modifiers |= Qt::MetaModifier;
        }

        m_ks = QKeySequence(QKeyCombination(modifiers, Qt::Key_Print));
        if (result != nullptr) {
            *result = 0;
        }
        if (msg->message == WM_KEYUP || msg->message == WM_SYSKEYUP) {
            QTimer::singleShot(0, this, &SetShortcutDialog::accept);
        }
        return true;
    }

    return QDialog::nativeEvent(eventType, message, result);
}
#endif

void SetShortcutDialog::accept()
{
    stopCapture();
    QDialog::accept();
}

void SetShortcutDialog::reject()
{
    stopCapture();
    QDialog::reject();
}
