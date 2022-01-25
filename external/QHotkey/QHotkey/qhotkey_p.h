#ifndef QHOTKEY_P_H
#define QHOTKEY_P_H

#include "qhotkey.h"
#include <QAbstractNativeEventFilter>
#include <QMultiHash>
#include <QMutex>
#include <QGlobalStatic>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	#define _NATIVE_EVENT_RESULT qintptr
#else
	#define _NATIVE_EVENT_RESULT long
#endif

class QHOTKEY_EXPORT QHotkeyPrivate : public QObject, public QAbstractNativeEventFilter
{
	Q_OBJECT

public:
	QHotkeyPrivate();//singleton!!!
	~QHotkeyPrivate();

	static QHotkeyPrivate *instance();
	static bool isPlatformSupported();

	QHotkey::NativeShortcut nativeShortcut(Qt::Key keycode, Qt::KeyboardModifiers modifiers);

	bool addShortcut(QHotkey *hotkey);
	bool removeShortcut(QHotkey *hotkey);

protected:
	void activateShortcut(QHotkey::NativeShortcut shortcut);
	void releaseShortcut(QHotkey::NativeShortcut shortcut);

	virtual quint32 nativeKeycode(Qt::Key keycode, bool &ok) = 0;//platform implement
	virtual quint32 nativeModifiers(Qt::KeyboardModifiers modifiers, bool &ok) = 0;//platform implement

	virtual bool registerShortcut(QHotkey::NativeShortcut shortcut) = 0;//platform implement
	virtual bool unregisterShortcut(QHotkey::NativeShortcut shortcut) = 0;//platform implement

	QString error;

private:
	QHash<QPair<Qt::Key, Qt::KeyboardModifiers>, QHotkey::NativeShortcut> mapping;
	QMultiHash<QHotkey::NativeShortcut, QHotkey*> shortcuts;

	Q_INVOKABLE void addMappingInvoked(Qt::Key keycode, Qt::KeyboardModifiers modifiers, QHotkey::NativeShortcut nativeShortcut);
	Q_INVOKABLE bool addShortcutInvoked(QHotkey *hotkey);
	Q_INVOKABLE bool removeShortcutInvoked(QHotkey *hotkey);
	Q_INVOKABLE QHotkey::NativeShortcut nativeShortcutInvoked(Qt::Key keycode, Qt::KeyboardModifiers modifiers);
};

#define NATIVE_INSTANCE(ClassName) \
	Q_GLOBAL_STATIC(ClassName, hotkeyPrivate) \
	\
	QHotkeyPrivate *QHotkeyPrivate::instance()\
	{\
		return hotkeyPrivate;\
	}

#endif // QHOTKEY_P_H
