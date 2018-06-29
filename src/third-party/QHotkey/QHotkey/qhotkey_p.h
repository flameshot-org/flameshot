#ifndef QHOTKEY_P_H
#define QHOTKEY_P_H

#include "qhotkey.h"
#include <QAbstractNativeEventFilter>
#include <QMultiHash>
#include <QMutex>
#include <QGlobalStatic>

class QHOTKEY_SHARED_EXPORT QHotkeyPrivate : public QObject, public QAbstractNativeEventFilter
{
	Q_OBJECT

public:
	QHotkeyPrivate();//singleton!!!
	~QHotkeyPrivate();

	static QHotkeyPrivate *instance();

	QHotkey::NativeShortcut nativeShortcut(Qt::Key keycode, Qt::KeyboardModifiers modifiers);

	bool addShortcut(QHotkey *hotkey);
	bool removeShortcut(QHotkey *hotkey);

protected:
	void activateShortcut(QHotkey::NativeShortcut shortcut);

	virtual quint32 nativeKeycode(Qt::Key keycode, bool &ok) = 0;//platform implement
	virtual quint32 nativeModifiers(Qt::KeyboardModifiers modifiers, bool &ok) = 0;//platform implement

	virtual bool registerShortcut(QHotkey::NativeShortcut shortcut) = 0;//platform implement
	virtual bool unregisterShortcut(QHotkey::NativeShortcut shortcut) = 0;//platform implement

private:
	QHash<QPair<Qt::Key, Qt::KeyboardModifiers>, QHotkey::NativeShortcut> mapping;
	QMultiHash<QHotkey::NativeShortcut, QHotkey*> shortcuts;

	Q_INVOKABLE void addMappingInvoked(Qt::Key keycode, Qt::KeyboardModifiers modifiers, const QHotkey::NativeShortcut &nativeShortcut);
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

Q_DECLARE_METATYPE(Qt::Key)
Q_DECLARE_METATYPE(Qt::KeyboardModifiers)

#endif // QHOTKEY_P_H
