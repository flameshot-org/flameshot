#include "qhotkey.h"
#include "qhotkey_p.h"
#include <Carbon/Carbon.h>
#include <QDebug>

class QHotkeyPrivateMac : public QHotkeyPrivate
{
public:
	// QAbstractNativeEventFilter interface
	bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;

	static OSStatus hotkeyEventHandler(EventHandlerCallRef nextHandler, EventRef event, void* data);

protected:
	// QHotkeyPrivate interface
	quint32 nativeKeycode(Qt::Key keycode, bool &ok) Q_DECL_OVERRIDE;
	quint32 nativeModifiers(Qt::KeyboardModifiers modifiers, bool &ok) Q_DECL_OVERRIDE;
	bool registerShortcut(QHotkey::NativeShortcut shortcut) Q_DECL_OVERRIDE;
	bool unregisterShortcut(QHotkey::NativeShortcut shortcut) Q_DECL_OVERRIDE;

private:
	static bool isHotkeyHandlerRegistered;
	static QHash<QHotkey::NativeShortcut, EventHotKeyRef> hotkeyRefs;
};
NATIVE_INSTANCE(QHotkeyPrivateMac)

bool QHotkeyPrivateMac::isHotkeyHandlerRegistered = false;
QHash<QHotkey::NativeShortcut, EventHotKeyRef> QHotkeyPrivateMac::hotkeyRefs;

bool QHotkeyPrivateMac::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
	Q_UNUSED(eventType);
	Q_UNUSED(message);
	Q_UNUSED(result);
	return false;
}

quint32 QHotkeyPrivateMac::nativeKeycode(Qt::Key keycode, bool &ok)
{
	// Constants found in NSEvent.h from AppKit.framework
	ok = true;
	switch (keycode) {
	case Qt::Key_Return:
		return kVK_Return;
	case Qt::Key_Enter:
		return kVK_ANSI_KeypadEnter;
	case Qt::Key_Tab:
		return kVK_Tab;
	case Qt::Key_Space:
		return kVK_Space;
	case Qt::Key_Backspace:
		return kVK_Delete;
	case Qt::Key_Escape:
		return kVK_Escape;
	case Qt::Key_CapsLock:
		return kVK_CapsLock;
	case Qt::Key_Option:
		return kVK_Option;
	case Qt::Key_F17:
		return kVK_F17;
	case Qt::Key_VolumeUp:
		return kVK_VolumeUp;
	case Qt::Key_VolumeDown:
		return kVK_VolumeDown;
	case Qt::Key_F18:
		return kVK_F18;
	case Qt::Key_F19:
		return kVK_F19;
	case Qt::Key_F20:
		return kVK_F20;
	case Qt::Key_F5:
		return kVK_F5;
	case Qt::Key_F6:
		return kVK_F6;
	case Qt::Key_F7:
		return kVK_F7;
	case Qt::Key_F3:
		return kVK_F3;
	case Qt::Key_F8:
		return kVK_F8;
	case Qt::Key_F9:
		return kVK_F9;
	case Qt::Key_F11:
		return kVK_F11;
	case Qt::Key_F13:
		return kVK_F13;
	case Qt::Key_F16:
		return kVK_F16;
	case Qt::Key_F14:
		return kVK_F14;
	case Qt::Key_F10:
		return kVK_F10;
	case Qt::Key_F12:
		return kVK_F12;
	case Qt::Key_F15:
		return kVK_F15;
	case Qt::Key_Help:
		return kVK_Help;
	case Qt::Key_Home:
		return kVK_Home;
	case Qt::Key_PageUp:
		return kVK_PageUp;
	case Qt::Key_Delete:
		return kVK_ForwardDelete;
	case Qt::Key_F4:
		return kVK_F4;
	case Qt::Key_End:
		return kVK_End;
	case Qt::Key_F2:
		return kVK_F2;
	case Qt::Key_PageDown:
		return kVK_PageDown;
	case Qt::Key_F1:
		return kVK_F1;
	case Qt::Key_Left:
		return kVK_LeftArrow;
	case Qt::Key_Right:
		return kVK_RightArrow;
	case Qt::Key_Down:
		return kVK_DownArrow;
	case Qt::Key_Up:
		return kVK_UpArrow;
	default:
		ok = false;
		break;
	}

	UTF16Char ch = keycode;

	CFDataRef currentLayoutData;
	TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();

	if (currentKeyboard == NULL)
		return 0;

	currentLayoutData = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
	CFRelease(currentKeyboard);
	if (currentLayoutData == NULL)
		return 0;

	UCKeyboardLayout* header = (UCKeyboardLayout*)CFDataGetBytePtr(currentLayoutData);
	UCKeyboardTypeHeader* table = header->keyboardTypeList;

	uint8_t *data = (uint8_t*)header;
	for (quint32 i=0; i < header->keyboardTypeCount; i++) {
		UCKeyStateRecordsIndex* stateRec = 0;
		if (table[i].keyStateRecordsIndexOffset != 0) {
			stateRec = reinterpret_cast<UCKeyStateRecordsIndex*>(data + table[i].keyStateRecordsIndexOffset);
			if (stateRec->keyStateRecordsIndexFormat != kUCKeyStateRecordsIndexFormat) stateRec = 0;
		}

		UCKeyToCharTableIndex* charTable = reinterpret_cast<UCKeyToCharTableIndex*>(data + table[i].keyToCharTableIndexOffset);
		if (charTable->keyToCharTableIndexFormat != kUCKeyToCharTableIndexFormat) continue;

		for (quint32 j=0; j < charTable->keyToCharTableCount; j++) {
			UCKeyOutput* keyToChar = reinterpret_cast<UCKeyOutput*>(data + charTable->keyToCharTableOffsets[j]);
			for (quint32 k=0; k < charTable->keyToCharTableSize; k++) {
				if (keyToChar[k] & kUCKeyOutputTestForIndexMask) {
					long idx = keyToChar[k] & kUCKeyOutputGetIndexMask;
					if (stateRec && idx < stateRec->keyStateRecordCount) {
						UCKeyStateRecord* rec = reinterpret_cast<UCKeyStateRecord*>(data + stateRec->keyStateRecordOffsets[idx]);
						if (rec->stateZeroCharData == ch) {
							ok = true;
							return k;
						}
					}
				}
				else if (!(keyToChar[k] & kUCKeyOutputSequenceIndexMask) && keyToChar[k] < 0xFFFE) {
					if (keyToChar[k] == ch) {
						ok = true;
						return k;
					}
				}
			}
		}
	}
	return 0;
}

quint32 QHotkeyPrivateMac::nativeModifiers(Qt::KeyboardModifiers modifiers, bool &ok)
{
	quint32 nMods = 0;
	if (modifiers & Qt::ShiftModifier)
		nMods |= shiftKey;
	if (modifiers & Qt::ControlModifier)
		nMods |= cmdKey;
	if (modifiers & Qt::AltModifier)
		nMods |= optionKey;
	if (modifiers & Qt::MetaModifier)
		nMods |= controlKey;
	if (modifiers & Qt::KeypadModifier)
		nMods |= kEventKeyModifierNumLockMask;
	ok = true;
	return nMods;
}

bool QHotkeyPrivateMac::registerShortcut(QHotkey::NativeShortcut shortcut)
{
	if (!this->isHotkeyHandlerRegistered)
	{
		EventTypeSpec eventSpec;
		eventSpec.eventClass = kEventClassKeyboard;
		eventSpec.eventKind = kEventHotKeyPressed;
		InstallApplicationEventHandler(&QHotkeyPrivateMac::hotkeyEventHandler, 1, &eventSpec, NULL, NULL);
	}

	EventHotKeyID hkeyID;
	hkeyID.signature = shortcut.key;
	hkeyID.id = shortcut.modifier;

	EventHotKeyRef eventRef = 0;
	OSStatus status = RegisterEventHotKey(shortcut.key,
										  shortcut.modifier,
										  hkeyID,
										  GetApplicationEventTarget(),
										  0,
										  &eventRef);
	if (status != noErr) {
		qCWarning(logQHotkey) << "Failed to register hotkey. Error:"
							  << status;
		return false;
	} else {
		this->hotkeyRefs.insert(shortcut, eventRef);
		return true;
	}
}

bool QHotkeyPrivateMac::unregisterShortcut(QHotkey::NativeShortcut shortcut)
{
	EventHotKeyRef eventRef = QHotkeyPrivateMac::hotkeyRefs.value(shortcut);
	OSStatus status = UnregisterEventHotKey(eventRef);
	if (status != noErr) {
		qCWarning(logQHotkey) << "Failed to unregister hotkey. Error:"
							  << status;
		return false;
	} else {
		this->hotkeyRefs.remove(shortcut);
		return true;
	}
}

OSStatus QHotkeyPrivateMac::hotkeyEventHandler(EventHandlerCallRef nextHandler, EventRef event, void* data)
{
	Q_UNUSED(nextHandler);
	Q_UNUSED(data);

	if (GetEventClass(event) == kEventClassKeyboard &&
		GetEventKind(event) == kEventHotKeyPressed) {
		EventHotKeyID hkeyID;
		GetEventParameter(event,
						  kEventParamDirectObject,
						  typeEventHotKeyID,
						  NULL,
						  sizeof(EventHotKeyID),
						  NULL,
						  &hkeyID);
		hotkeyPrivate->activateShortcut({hkeyID.signature, hkeyID.id});
	}

	return noErr;
}
