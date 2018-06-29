#include "qhotkey.h"
#include "qhotkey_p.h"
#include <QDebug>
#include <QX11Info>
#include <QThreadStorage>
#include <X11/Xlib.h>
#include <xcb/xcb.h>

//compability to pre Qt 5.8
#ifndef Q_FALLTHROUGH
#define Q_FALLTHROUGH() (void)0
#endif

class QHotkeyPrivateX11 : public QHotkeyPrivate
{
public:
	// QAbstractNativeEventFilter interface
	bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;

protected:
	// QHotkeyPrivate interface
	quint32 nativeKeycode(Qt::Key keycode, bool &ok) Q_DECL_OVERRIDE;
	quint32 nativeModifiers(Qt::KeyboardModifiers modifiers, bool &ok) Q_DECL_OVERRIDE;
	QString getX11String(Qt::Key keycode);
	bool registerShortcut(QHotkey::NativeShortcut shortcut) Q_DECL_OVERRIDE;
	bool unregisterShortcut(QHotkey::NativeShortcut shortcut) Q_DECL_OVERRIDE;

private:
	static const QVector<quint32> specialModifiers;
	static const quint32 validModsMask;

	static QString formatX11Error(Display *display, int errorCode);

	class HotkeyErrorHandler {
	public:
		HotkeyErrorHandler();
		~HotkeyErrorHandler();

		static bool hasError;
		static QString errorString;

	private:
		XErrorHandler prevHandler;

		static int handleError(Display *display, XErrorEvent *error);
	};
};
NATIVE_INSTANCE(QHotkeyPrivateX11)

const QVector<quint32> QHotkeyPrivateX11::specialModifiers = {0, Mod2Mask, LockMask, (Mod2Mask | LockMask)};
const quint32 QHotkeyPrivateX11::validModsMask = ShiftMask | ControlMask | Mod1Mask | Mod4Mask;

bool QHotkeyPrivateX11::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
	Q_UNUSED(eventType);
	Q_UNUSED(result);

	xcb_generic_event_t *genericEvent = static_cast<xcb_generic_event_t *>(message);
	if (genericEvent->response_type == XCB_KEY_PRESS) {
		xcb_key_press_event_t *keyEvent = static_cast<xcb_key_press_event_t *>(message);
		this->activateShortcut({keyEvent->detail, keyEvent->state & QHotkeyPrivateX11::validModsMask});
	}

	return false;
}

QString QHotkeyPrivateX11::getX11String(Qt::Key keycode)
{
	switch(keycode){

		case Qt::Key_MediaLast : 
		case Qt::Key_MediaPrevious : 
			return "XF86AudioPrev";
		case Qt::Key_MediaNext : 
			return "XF86AudioNext";
		case Qt::Key_MediaPause : 
		case Qt::Key_MediaPlay : 
		case Qt::Key_MediaTogglePlayPause :
			return "XF86AudioPlay";
		case Qt::Key_MediaRecord :
			return "XF86AudioRecord"; 
		case Qt::Key_MediaStop : 
			return "XF86AudioStop";
		default :
			return QKeySequence(keycode).toString(QKeySequence::NativeText);
	}
}

quint32 QHotkeyPrivateX11::nativeKeycode(Qt::Key keycode, bool &ok)
{
	QString keyString = getX11String(keycode);

	KeySym keysym = XStringToKeysym(keyString.toLatin1().constData());
	if (keysym == NoSymbol) {
		//not found -> just use the key
		if(keycode <= 0xFFFF)
			keysym = keycode;
		else
			return 0;
	}

	Display *display = QX11Info::display();
	if(display) {
		auto res = XKeysymToKeycode(QX11Info::display(), keysym);
		if(res != 0)
			ok = true;
		return res;
	} else
		return 0;
}

quint32 QHotkeyPrivateX11::nativeModifiers(Qt::KeyboardModifiers modifiers, bool &ok)
{
	quint32 nMods = 0;
	if (modifiers & Qt::ShiftModifier)
		nMods |= ShiftMask;
	if (modifiers & Qt::ControlModifier)
		nMods |= ControlMask;
	if (modifiers & Qt::AltModifier)
		nMods |= Mod1Mask;
	if (modifiers & Qt::MetaModifier)
		nMods |= Mod4Mask;
	ok = true;
	return nMods;
}

bool QHotkeyPrivateX11::registerShortcut(QHotkey::NativeShortcut shortcut)
{
	Display *display = QX11Info::display();
	if(!display)
		return false;

	HotkeyErrorHandler errorHandler;
	for(quint32 specialMod : QHotkeyPrivateX11::specialModifiers) {
		XGrabKey(display,
				 shortcut.key,
				 shortcut.modifier | specialMod,
				 DefaultRootWindow(display),
				 True,
				 GrabModeAsync,
				 GrabModeAsync);
	}
	XSync(display, False);

	if(errorHandler.hasError) {
		qCWarning(logQHotkey) << "Failed to register hotkey. Error:"
							  << qPrintable(errorHandler.errorString);
		this->unregisterShortcut(shortcut);
		return false;
	} else
		return true;
}

bool QHotkeyPrivateX11::unregisterShortcut(QHotkey::NativeShortcut shortcut)
{
	Display *display = QX11Info::display();
	if(!display)
		return false;

	HotkeyErrorHandler errorHandler;
	for(quint32 specialMod : QHotkeyPrivateX11::specialModifiers) {
		XUngrabKey(display,
				   shortcut.key,
				   shortcut.modifier | specialMod,
				   DefaultRootWindow(display));
	}
	XSync(display, False);

	if(errorHandler.hasError) {
		qCWarning(logQHotkey) << "Failed to unregister hotkey. Error:"
							  << qPrintable(errorHandler.errorString);
		return false;
	} else
		return true;
}

QString QHotkeyPrivateX11::formatX11Error(Display *display, int errorCode)
{
	char errStr[256];
	XGetErrorText(display, errorCode, errStr, 256);
	return QString::fromLatin1(errStr);
}



// ---------- QHotkeyPrivateX11::HotkeyErrorHandler implementation ----------

bool QHotkeyPrivateX11::HotkeyErrorHandler::hasError = false;
QString QHotkeyPrivateX11::HotkeyErrorHandler::errorString;

QHotkeyPrivateX11::HotkeyErrorHandler::HotkeyErrorHandler()
{
	prevHandler = XSetErrorHandler(&HotkeyErrorHandler::handleError);
}

QHotkeyPrivateX11::HotkeyErrorHandler::~HotkeyErrorHandler()
{
	XSetErrorHandler(prevHandler);
	hasError = false;
	errorString.clear();
}

int QHotkeyPrivateX11::HotkeyErrorHandler::handleError(Display *display, XErrorEvent *error)
{
	switch (error->error_code) {
	case BadAccess:
	case BadValue:
	case BadWindow:
		if (error->request_code == 33 || //grab key
			error->request_code == 34) {// ungrab key
			hasError = true;
			errorString = QHotkeyPrivateX11::formatX11Error(display, error->error_code);
			return 1;
		}
		Q_FALLTHROUGH();
		// fall through
	default:
		return 0;
	}
}
