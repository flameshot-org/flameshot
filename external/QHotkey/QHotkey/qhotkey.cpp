#include "qhotkey.h"
#include "qhotkey_p.h"
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QMetaMethod>
#include <QThread>
#include <QDebug>

Q_LOGGING_CATEGORY(logQHotkey, "QHotkey")

void QHotkey::addGlobalMapping(const QKeySequence &shortcut, QHotkey::NativeShortcut nativeShortcut)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	const int key = shortcut[0].toCombined();
#else
	const int key = shortcut[0];
#endif

	QMetaObject::invokeMethod(QHotkeyPrivate::instance(), "addMappingInvoked", Qt::QueuedConnection,
							  Q_ARG(Qt::Key, Qt::Key(key & ~Qt::KeyboardModifierMask)),
							  Q_ARG(Qt::KeyboardModifiers, Qt::KeyboardModifiers(key & Qt::KeyboardModifierMask)),
							  Q_ARG(QHotkey::NativeShortcut, nativeShortcut));
}

bool QHotkey::isPlatformSupported()
{
	return QHotkeyPrivate::isPlatformSupported();
}

QHotkey::QHotkey(QObject *parent) :
	QObject(parent),
	_keyCode(Qt::Key_unknown),
	_modifiers(Qt::NoModifier),
	_registered(false)
{}

QHotkey::QHotkey(const QKeySequence &shortcut, bool autoRegister, QObject *parent) :
	QHotkey(parent)
{
	setShortcut(shortcut, autoRegister);
}

QHotkey::QHotkey(Qt::Key keyCode, Qt::KeyboardModifiers modifiers, bool autoRegister, QObject *parent) :
	QHotkey(parent)
{
	setShortcut(keyCode, modifiers, autoRegister);
}

QHotkey::QHotkey(QHotkey::NativeShortcut shortcut, bool autoRegister, QObject *parent) :
	QHotkey(parent)
{
	setNativeShortcut(shortcut, autoRegister);
}

QHotkey::~QHotkey()
{
	if(_registered)
		QHotkeyPrivate::instance()->removeShortcut(this);
}

QKeySequence QHotkey::shortcut() const
{
	if(_keyCode == Qt::Key_unknown)
		return QKeySequence();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	return QKeySequence((_keyCode | _modifiers).toCombined());
#else
	return QKeySequence(static_cast<int>(_keyCode | _modifiers));
#endif
}

Qt::Key QHotkey::keyCode() const
{
	return _keyCode;
}

Qt::KeyboardModifiers QHotkey::modifiers() const
{
	return _modifiers;
}

QHotkey::NativeShortcut QHotkey::currentNativeShortcut() const
{
	return _nativeShortcut;
}

bool QHotkey::isRegistered() const
{
	return _registered;
}

bool QHotkey::setShortcut(const QKeySequence &shortcut, bool autoRegister)
{
	if(shortcut.isEmpty())
		return resetShortcut();
	if(shortcut.count() > 1) {
		qCWarning(logQHotkey, "Keysequences with multiple shortcuts are not allowed! "
							  "Only the first shortcut will be used!");
	}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	const int key = shortcut[0].toCombined();
#else
	const int key = shortcut[0];
#endif

	return setShortcut(Qt::Key(key & ~Qt::KeyboardModifierMask),
			Qt::KeyboardModifiers(key & Qt::KeyboardModifierMask),
			autoRegister);
}

bool QHotkey::setShortcut(Qt::Key keyCode, Qt::KeyboardModifiers modifiers, bool autoRegister)
{
	if(_registered) {
		if(autoRegister) {
			if(!QHotkeyPrivate::instance()->removeShortcut(this))
				return false;
		} else
			return false;
	}

	if(keyCode == Qt::Key_unknown) {
		_keyCode = Qt::Key_unknown;
		_modifiers = Qt::NoModifier;
		_nativeShortcut = NativeShortcut();
		return true;
	}

	_keyCode = keyCode;
	_modifiers = modifiers;
	_nativeShortcut = QHotkeyPrivate::instance()->nativeShortcut(keyCode, modifiers);
	if(_nativeShortcut.isValid()) {
		if(autoRegister)
			return QHotkeyPrivate::instance()->addShortcut(this);
		return true;
	}

	qCWarning(logQHotkey) << "Unable to map shortcut to native keys. Key:" << keyCode << "Modifiers:" << modifiers;
	_keyCode = Qt::Key_unknown;
	_modifiers = Qt::NoModifier;
	_nativeShortcut = NativeShortcut();
	return false;
}

bool QHotkey::resetShortcut()
{
	if(_registered &&
	   !QHotkeyPrivate::instance()->removeShortcut(this)) {
		return false;
	}

	_keyCode = Qt::Key_unknown;
	_modifiers = Qt::NoModifier;
	_nativeShortcut = NativeShortcut();
	return true;
}

bool QHotkey::setNativeShortcut(QHotkey::NativeShortcut nativeShortcut, bool autoRegister)
{
	if(_registered) {
		if(autoRegister) {
			if(!QHotkeyPrivate::instance()->removeShortcut(this))
				return false;
		} else
			return false;
	}

	if(nativeShortcut.isValid()) {
		_keyCode = Qt::Key_unknown;
		_modifiers = Qt::NoModifier;
		_nativeShortcut = nativeShortcut;
		if(autoRegister)
			return QHotkeyPrivate::instance()->addShortcut(this);
		return true;
	} 

	_keyCode = Qt::Key_unknown;
	_modifiers = Qt::NoModifier;
	_nativeShortcut = NativeShortcut();
	return true;
}

bool QHotkey::setRegistered(bool registered)
{
	if(_registered && !registered)
		return QHotkeyPrivate::instance()->removeShortcut(this);
	if(!_registered && registered) {
		if(!_nativeShortcut.isValid())
			return false;
		return QHotkeyPrivate::instance()->addShortcut(this);
	}
	return true;
}



// ---------- QHotkeyPrivate implementation ----------

QHotkeyPrivate::QHotkeyPrivate()
{
	Q_ASSERT_X(qApp, Q_FUNC_INFO, "QHotkey requires QCoreApplication to be instantiated");
	qApp->eventDispatcher()->installNativeEventFilter(this);
}

QHotkeyPrivate::~QHotkeyPrivate()
{
	if(!shortcuts.isEmpty())
		qCWarning(logQHotkey) << "QHotkeyPrivate destroyed with registered shortcuts!";
	if(qApp && qApp->eventDispatcher())
		qApp->eventDispatcher()->removeNativeEventFilter(this);
}

QHotkey::NativeShortcut QHotkeyPrivate::nativeShortcut(Qt::Key keycode, Qt::KeyboardModifiers modifiers)
{
	Qt::ConnectionType conType = (QThread::currentThread() == thread() ?
									  Qt::DirectConnection :
									  Qt::BlockingQueuedConnection);
	QHotkey::NativeShortcut res;
	if(!QMetaObject::invokeMethod(this, "nativeShortcutInvoked", conType,
								  Q_RETURN_ARG(QHotkey::NativeShortcut, res),
								  Q_ARG(Qt::Key, keycode),
								  Q_ARG(Qt::KeyboardModifiers, modifiers))) {
		return QHotkey::NativeShortcut();
	}
	return res;
}

bool QHotkeyPrivate::addShortcut(QHotkey *hotkey)
{
	if(hotkey->_registered)
		return false;

	Qt::ConnectionType conType = (QThread::currentThread() == thread() ?
									  Qt::DirectConnection :
									  Qt::BlockingQueuedConnection);
	bool res = false;
	if(!QMetaObject::invokeMethod(this, "addShortcutInvoked", conType,
								  Q_RETURN_ARG(bool, res),
								  Q_ARG(QHotkey*, hotkey))) {
		return false;
	}

	if(res)
		emit hotkey->registeredChanged(true);
	return res;
}

bool QHotkeyPrivate::removeShortcut(QHotkey *hotkey)
{
	if(!hotkey->_registered)
		return false;

	Qt::ConnectionType conType = (QThread::currentThread() == thread() ?
									  Qt::DirectConnection :
									  Qt::BlockingQueuedConnection);
	bool res = false;
	if(!QMetaObject::invokeMethod(this, "removeShortcutInvoked", conType,
								  Q_RETURN_ARG(bool, res),
								  Q_ARG(QHotkey*, hotkey))) {
		return false;
	}

	if(res)
		emit hotkey->registeredChanged(false);
	return res;
}

void QHotkeyPrivate::activateShortcut(QHotkey::NativeShortcut shortcut)
{
	QMetaMethod signal = QMetaMethod::fromSignal(&QHotkey::activated);
	for(QHotkey *hkey : shortcuts.values(shortcut))
		signal.invoke(hkey, Qt::QueuedConnection);
}

void QHotkeyPrivate::releaseShortcut(QHotkey::NativeShortcut shortcut)
{
	QMetaMethod signal = QMetaMethod::fromSignal(&QHotkey::released);
	for(QHotkey *hkey : shortcuts.values(shortcut))
		signal.invoke(hkey, Qt::QueuedConnection);
}

void QHotkeyPrivate::addMappingInvoked(Qt::Key keycode, Qt::KeyboardModifiers modifiers, QHotkey::NativeShortcut nativeShortcut)
{
	mapping.insert({keycode, modifiers}, nativeShortcut);
}

bool QHotkeyPrivate::addShortcutInvoked(QHotkey *hotkey)
{
	QHotkey::NativeShortcut shortcut = hotkey->_nativeShortcut;

	if(!shortcuts.contains(shortcut)) {
		if(!registerShortcut(shortcut)) {
			qCWarning(logQHotkey) << QHotkey::tr("Failed to register %1. Error: %2").arg(hotkey->shortcut().toString(), error);
			return false;
		}
	}

	shortcuts.insert(shortcut, hotkey);
	hotkey->_registered = true;
	return true;
}

bool QHotkeyPrivate::removeShortcutInvoked(QHotkey *hotkey)
{
	QHotkey::NativeShortcut shortcut = hotkey->_nativeShortcut;

	if(shortcuts.remove(shortcut, hotkey) == 0)
		return false;
	hotkey->_registered = false;
	emit hotkey->registeredChanged(true);
	if(shortcuts.count(shortcut) == 0) {
		if (!unregisterShortcut(shortcut)) {
			qCWarning(logQHotkey) << QHotkey::tr("Failed to unregister %1. Error: %2").arg(hotkey->shortcut().toString(), error);
			return false;
		}
		return true;
	}
	return true;
}

QHotkey::NativeShortcut QHotkeyPrivate::nativeShortcutInvoked(Qt::Key keycode, Qt::KeyboardModifiers modifiers)
{
	if(mapping.contains({keycode, modifiers}))
		return mapping.value({keycode, modifiers});

	bool ok1 = false;
	auto k = nativeKeycode(keycode, ok1);
	bool ok2 = false;
	auto m = nativeModifiers(modifiers, ok2);
	if(ok1 && ok2)
		return {k, m};
	return {};
}



QHotkey::NativeShortcut::NativeShortcut() :
	key(),
	modifier(),
	valid(false)
{}

QHotkey::NativeShortcut::NativeShortcut(quint32 key, quint32 modifier) :
	key(key),
	modifier(modifier),
	valid(true)
{}

bool QHotkey::NativeShortcut::isValid() const
{
	return valid;
}

bool QHotkey::NativeShortcut::operator ==(QHotkey::NativeShortcut other) const
{
	return (key == other.key) &&
		   (modifier == other.modifier) &&
		   valid == other.valid;
}

bool QHotkey::NativeShortcut::operator !=(QHotkey::NativeShortcut other) const
{
	return (key != other.key) ||
		   (modifier != other.modifier) ||
		   valid != other.valid;
}

QHOTKEY_HASH_SEED qHash(QHotkey::NativeShortcut key)
{
	return qHash(key.key) ^ qHash(key.modifier);
}

QHOTKEY_HASH_SEED qHash(QHotkey::NativeShortcut key, QHOTKEY_HASH_SEED seed)
{
	return qHash(key.key, seed) ^ qHash(key.modifier, seed);
}
