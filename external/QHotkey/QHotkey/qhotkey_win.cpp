#include "qhotkey.h"
#include "qhotkey_p.h"
#include <qt_windows.h>
#include <QDebug>
#include <QTimer>

#define HKEY_ID(nativeShortcut) (((nativeShortcut.key ^ (nativeShortcut.modifier << 8)) & 0x0FFF) | 0x7000)

#if !defined(MOD_NOREPEAT)
#define MOD_NOREPEAT 0x4000
#endif

class QHotkeyPrivateWin : public QHotkeyPrivate
{
public:
	QHotkeyPrivateWin();
	// QAbstractNativeEventFilter interface
	bool nativeEventFilter(const QByteArray &eventType, void *message, _NATIVE_EVENT_RESULT *result) override;

protected:
	void pollForHotkeyRelease();
	// QHotkeyPrivate interface
	quint32 nativeKeycode(Qt::Key keycode, bool &ok) Q_DECL_OVERRIDE;
	quint32 nativeModifiers(Qt::KeyboardModifiers modifiers, bool &ok) Q_DECL_OVERRIDE;
	bool registerShortcut(QHotkey::NativeShortcut shortcut) Q_DECL_OVERRIDE;
	bool unregisterShortcut(QHotkey::NativeShortcut shortcut) Q_DECL_OVERRIDE;

private:
	static QString formatWinError(DWORD winError);
	QTimer pollTimer;
	QHotkey::NativeShortcut polledShortcut;
};
NATIVE_INSTANCE(QHotkeyPrivateWin)

QHotkeyPrivateWin::QHotkeyPrivateWin(){
	pollTimer.setInterval(50);
	connect(&pollTimer, &QTimer::timeout, this, &QHotkeyPrivateWin::pollForHotkeyRelease);
}

bool QHotkeyPrivate::isPlatformSupported()
{
	return true;
}

bool QHotkeyPrivateWin::nativeEventFilter(const QByteArray &eventType, void *message, _NATIVE_EVENT_RESULT *result)
{
	Q_UNUSED(eventType)
	Q_UNUSED(result)

	MSG* msg = static_cast<MSG*>(message);
	if(msg->message == WM_HOTKEY) {
		QHotkey::NativeShortcut shortcut = {HIWORD(msg->lParam), LOWORD(msg->lParam)};
		this->activateShortcut(shortcut);
		this->polledShortcut = shortcut;
		this->pollTimer.start();
	}

	return false;
}

void QHotkeyPrivateWin::pollForHotkeyRelease()
{
	bool pressed = (GetAsyncKeyState(this->polledShortcut.key) & (1 << 15)) != 0;
	if(!pressed) {
		this->pollTimer.stop();
		this->releaseShortcut(this->polledShortcut);
	}
}

quint32 QHotkeyPrivateWin::nativeKeycode(Qt::Key keycode, bool &ok)
{
	ok = true;
	if(keycode <= 0xFFFF) {//Try to obtain the key from it's "character"
		const SHORT vKey = VkKeyScanW(static_cast<WCHAR>(keycode));
		if(vKey > -1)
			return LOBYTE(vKey);
	}

	//find key from switch/case --> Only finds a very small subset of keys
	switch (keycode)
	{
	case Qt::Key_Escape:
		return VK_ESCAPE;
	case Qt::Key_Tab:
	case Qt::Key_Backtab:
		return VK_TAB;
	case Qt::Key_Backspace:
		return VK_BACK;
	case Qt::Key_Return:
	case Qt::Key_Enter:
		return VK_RETURN;
	case Qt::Key_Insert:
		return VK_INSERT;
	case Qt::Key_Delete:
		return VK_DELETE;
	case Qt::Key_Pause:
		return VK_PAUSE;
	case Qt::Key_Print:
		return VK_PRINT;
	case Qt::Key_Clear:
		return VK_CLEAR;
	case Qt::Key_Home:
		return VK_HOME;
	case Qt::Key_End:
		return VK_END;
	case Qt::Key_Left:
		return VK_LEFT;
	case Qt::Key_Up:
		return VK_UP;
	case Qt::Key_Right:
		return VK_RIGHT;
	case Qt::Key_Down:
		return VK_DOWN;
	case Qt::Key_PageUp:
		return VK_PRIOR;
	case Qt::Key_PageDown:
		return VK_NEXT;
	case Qt::Key_CapsLock:
		return VK_CAPITAL;
	case Qt::Key_NumLock:
		return VK_NUMLOCK;
	case Qt::Key_ScrollLock:
		return VK_SCROLL;

	case Qt::Key_F1:
		return VK_F1;
	case Qt::Key_F2:
		return VK_F2;
	case Qt::Key_F3:
		return VK_F3;
	case Qt::Key_F4:
		return VK_F4;
	case Qt::Key_F5:
		return VK_F5;
	case Qt::Key_F6:
		return VK_F6;
	case Qt::Key_F7:
		return VK_F7;
	case Qt::Key_F8:
		return VK_F8;
	case Qt::Key_F9:
		return VK_F9;
	case Qt::Key_F10:
		return VK_F10;
	case Qt::Key_F11:
		return VK_F11;
	case Qt::Key_F12:
		return VK_F12;
	case Qt::Key_F13:
		return VK_F13;
	case Qt::Key_F14:
		return VK_F14;
	case Qt::Key_F15:
		return VK_F15;
	case Qt::Key_F16:
		return VK_F16;
	case Qt::Key_F17:
		return VK_F17;
	case Qt::Key_F18:
		return VK_F18;
	case Qt::Key_F19:
		return VK_F19;
	case Qt::Key_F20:
		return VK_F20;
	case Qt::Key_F21:
		return VK_F21;
	case Qt::Key_F22:
		return VK_F22;
	case Qt::Key_F23:
		return VK_F23;
	case Qt::Key_F24:
		return VK_F24;

	case Qt::Key_Menu:
		return VK_APPS;
	case Qt::Key_Help:
		return VK_HELP;
	case Qt::Key_MediaNext:
		return VK_MEDIA_NEXT_TRACK;
	case Qt::Key_MediaPrevious:
		return VK_MEDIA_PREV_TRACK;
	case Qt::Key_MediaPlay:
		return VK_MEDIA_PLAY_PAUSE;
	case Qt::Key_MediaStop:
		return VK_MEDIA_STOP;
	case Qt::Key_VolumeDown:
		return VK_VOLUME_DOWN;
	case Qt::Key_VolumeUp:
		return VK_VOLUME_UP;
	case Qt::Key_VolumeMute:
		return VK_VOLUME_MUTE;
	case Qt::Key_Mode_switch:
		return VK_MODECHANGE;
	case Qt::Key_Select:
		return VK_SELECT;
	case Qt::Key_Printer:
		return VK_PRINT;
	case Qt::Key_Execute:
		return VK_EXECUTE;
	case Qt::Key_Sleep:
		return VK_SLEEP;
	case Qt::Key_Period:
		return VK_DECIMAL;
	case Qt::Key_Play:
		return VK_PLAY;
	case Qt::Key_Cancel:
		return VK_CANCEL;

	case Qt::Key_Forward:
		return VK_BROWSER_FORWARD;
	case Qt::Key_Refresh:
		return VK_BROWSER_REFRESH;
	case Qt::Key_Stop:
		return VK_BROWSER_STOP;
	case Qt::Key_Search:
		return VK_BROWSER_SEARCH;
	case Qt::Key_Favorites:
		return VK_BROWSER_FAVORITES;
	case Qt::Key_HomePage:
		return VK_BROWSER_HOME;

	case Qt::Key_LaunchMail:
		return VK_LAUNCH_MAIL;
	case Qt::Key_LaunchMedia:
		return VK_LAUNCH_MEDIA_SELECT;
	case Qt::Key_Launch0:
		return VK_LAUNCH_APP1;
	case Qt::Key_Launch1:
		return VK_LAUNCH_APP2;

	case Qt::Key_Massyo:
		return VK_OEM_FJ_MASSHOU;
	case Qt::Key_Touroku:
		return VK_OEM_FJ_TOUROKU;

	default:
		if(keycode <= 0xFFFF)
			return (byte)keycode;
		else {
			ok = false;
			return 0;
		}
	}
}

quint32 QHotkeyPrivateWin::nativeModifiers(Qt::KeyboardModifiers modifiers, bool &ok)
{
	quint32 nMods = 0;
	if (modifiers & Qt::ShiftModifier)
		nMods |= MOD_SHIFT;
	if (modifiers & Qt::ControlModifier)
		nMods |= MOD_CONTROL;
	if (modifiers & Qt::AltModifier)
		nMods |= MOD_ALT;
	if (modifiers & Qt::MetaModifier)
		nMods |= MOD_WIN;
	ok = true;
	return nMods;
}

bool QHotkeyPrivateWin::registerShortcut(QHotkey::NativeShortcut shortcut)
{
	BOOL ok = RegisterHotKey(NULL,
							 HKEY_ID(shortcut),
							 shortcut.modifier + MOD_NOREPEAT,
							 shortcut.key);
	if(ok)
		return true;
	else {
		error = QHotkeyPrivateWin::formatWinError(::GetLastError());
		return false;
	}
}

bool QHotkeyPrivateWin::unregisterShortcut(QHotkey::NativeShortcut shortcut)
{
	BOOL ok = UnregisterHotKey(NULL, HKEY_ID(shortcut));
	if(ok)
		return true;
	else {
		error = QHotkeyPrivateWin::formatWinError(::GetLastError());
		return false;
	}
}

QString QHotkeyPrivateWin::formatWinError(DWORD winError)
{
	wchar_t *buffer = NULL;
	DWORD num = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							   NULL,
							   winError,
							   0,
							   (LPWSTR)&buffer,
							   0,
							   NULL);
	if(buffer) {
		QString res = QString::fromWCharArray(buffer, num);
		LocalFree(buffer);
		return res;
	} else
		return QString();
}
