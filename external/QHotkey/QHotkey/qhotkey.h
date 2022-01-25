#ifndef QHOTKEY_H
#define QHOTKEY_H

#include <QObject>
#include <QKeySequence>
#include <QPair>
#include <QLoggingCategory>

#ifdef QHOTKEY_SHARED
#	ifdef QHOTKEY_LIBRARY
#		define QHOTKEY_EXPORT Q_DECL_EXPORT
#	else
#		define QHOTKEY_EXPORT Q_DECL_IMPORT
#	endif
#else
#	define QHOTKEY_EXPORT
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	#define QHOTKEY_HASH_SEED size_t
#else
	#define QHOTKEY_HASH_SEED uint
#endif

//! A class to define global, systemwide Hotkeys
class QHOTKEY_EXPORT QHotkey : public QObject
{
	Q_OBJECT
	//! @private
	friend class QHotkeyPrivate;

	//! Specifies whether this hotkey is currently registered or not
	Q_PROPERTY(bool registered READ isRegistered WRITE setRegistered NOTIFY registeredChanged)
	//! Holds the shortcut this hotkey will be triggered on
	Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut RESET resetShortcut)

public:
	//! Defines shortcut with native keycodes
	class QHOTKEY_EXPORT NativeShortcut {
	public:
		//! The native keycode
		quint32 key;
		//! The native modifiers
		quint32 modifier;

		//! Creates an invalid native shortcut
		NativeShortcut();
		//! Creates a valid native shortcut, with the given key and modifiers
		NativeShortcut(quint32 key, quint32 modifier = 0);

		//! Checks, whether this shortcut is valid or not
		bool isValid() const;

		//! Equality operator
		bool operator ==(NativeShortcut other) const;
		//! Inequality operator
		bool operator !=(NativeShortcut other) const;

	private:
		bool valid;
	};

	//! Adds a global mapping of a key sequence to a replacement native shortcut
	static void addGlobalMapping(const QKeySequence &shortcut, NativeShortcut nativeShortcut);

	//! Checks if global shortcuts are supported by the current platform
	static bool isPlatformSupported();

	//! Default Constructor
	explicit QHotkey(QObject *parent = nullptr);
	//! Constructs a hotkey with a shortcut and optionally registers it
	explicit QHotkey(const QKeySequence &shortcut, bool autoRegister = false, QObject *parent = nullptr);
	//! Constructs a hotkey with a key and modifiers and optionally registers it
	explicit QHotkey(Qt::Key keyCode, Qt::KeyboardModifiers modifiers, bool autoRegister = false, QObject *parent = nullptr);
	//! Constructs a hotkey from a native shortcut and optionally registers it
	explicit QHotkey(NativeShortcut shortcut, bool autoRegister = false, QObject *parent = nullptr);
	~QHotkey() override;

	//! @readAcFn{QHotkey::registered}
	bool isRegistered() const;
	//! @readAcFn{QHotkey::shortcut}
	QKeySequence shortcut() const;
	//! @readAcFn{QHotkey::shortcut} - the key only
	Qt::Key keyCode() const;
	//! @readAcFn{QHotkey::shortcut} - the modifiers only
	Qt::KeyboardModifiers modifiers() const;

	//! Get the current native shortcut
	NativeShortcut currentNativeShortcut() const;

public slots:
	//! @writeAcFn{QHotkey::registered}
	bool setRegistered(bool registered);

	//! @writeAcFn{QHotkey::shortcut}
	bool setShortcut(const QKeySequence &shortcut, bool autoRegister = false);
	//! @writeAcFn{QHotkey::shortcut}
	bool setShortcut(Qt::Key keyCode, Qt::KeyboardModifiers modifiers, bool autoRegister = false);
	//! @resetAcFn{QHotkey::shortcut}
	bool resetShortcut();

	//! Set this hotkey to a native shortcut
	bool setNativeShortcut(QHotkey::NativeShortcut nativeShortcut, bool autoRegister = false);

signals:
	//! Will be emitted if the shortcut is pressed
	void activated(QPrivateSignal);

	//! Will be emitted if the shortcut press is released
	void released(QPrivateSignal);

	//! @notifyAcFn{QHotkey::registered}
	void registeredChanged(bool registered);

private:
	Qt::Key _keyCode;
	Qt::KeyboardModifiers _modifiers;

	NativeShortcut _nativeShortcut;
	bool _registered;
};

QHOTKEY_HASH_SEED QHOTKEY_EXPORT qHash(QHotkey::NativeShortcut key);
QHOTKEY_HASH_SEED QHOTKEY_EXPORT qHash(QHotkey::NativeShortcut key, QHOTKEY_HASH_SEED seed);

QHOTKEY_EXPORT Q_DECLARE_LOGGING_CATEGORY(logQHotkey)

Q_DECLARE_METATYPE(QHotkey::NativeShortcut)

#endif // QHOTKEY_H
