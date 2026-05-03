import Gio from 'gi://Gio';
import GLib from 'gi://GLib';

import {Extension} from 'resource:///org/gnome/shell/extensions/extension.js';
import * as Main from 'resource:///org/gnome/shell/ui/main.js';

const BUS_NAME = 'org.flameshot.ShellIntegration';
const OBJECT_PATH = '/org/flameshot/ShellIntegration';

const INTERFACE_XML = `
<node>
  <interface name="${BUS_NAME}">
    <method name="Screenshot">
      <arg name="filename" type="s" direction="in"/>
      <arg name="success" type="b" direction="out"/>
      <arg name="filename_used" type="s" direction="out"/>
      <arg name="pointer_x" type="i" direction="out"/>
      <arg name="pointer_y" type="i" direction="out"/>
    </method>
  </interface>
</node>`;

export default class FlameshotNativeExtension extends Extension {
    enable() {
        this._allowlistOwner = null;
        this._watcherId = 0;

        this._dbusObject = Gio.DBusExportedObject.wrapJSObject(
            INTERFACE_XML, this);
        this._dbusObject.export(Gio.DBus.session, OBJECT_PATH);
        this._ownName = Gio.DBus.session.own_name(
            BUS_NAME, Gio.BusNameOwnerFlags.NONE, null, null);
        this._watcherId = Gio.DBus.watch_name(
            Gio.BusType.SESSION,
            BUS_NAME,
            Gio.BusNameWatcherFlags.NONE,
            (connection_, name_, owner) => this._allowScreenshotOwner(owner),
            () => this._removeScreenshotOwner());
    }

    disable() {
        if (this._watcherId) {
            Gio.DBus.unwatch_name(this._watcherId);
            this._watcherId = 0;
        }
        this._removeScreenshotOwner();

        if (this._ownName) {
            Gio.DBus.session.unown_name(this._ownName);
            this._ownName = 0;
        }

        if (this._dbusObject) {
            this._dbusObject.unexport();
            this._dbusObject.run_dispose();
            this._dbusObject = null;
        }
    }

    _senderChecker() {
        return Main.shellDBusService?._screenshotService?._senderChecker ?? null;
    }

    _allowScreenshotOwner(owner) {
        const checker = this._senderChecker();
        if (!checker)
            return;

        this._allowlistOwner = owner;
        checker._allowlistMap.set(BUS_NAME, owner);
        checker._checkAndResolveInitialized(BUS_NAME);
    }

    _removeScreenshotOwner() {
        const checker = this._senderChecker();
        if (!checker || !this._allowlistOwner)
            return;

        checker._allowlistMap.delete(BUS_NAME);
        checker._checkAndResolveInitialized(BUS_NAME);
        this._allowlistOwner = null;
    }

    ScreenshotAsync([filename], invocation) {
        const [pointerX, pointerY] = global.get_pointer();

        Gio.DBus.session.call(
            'org.gnome.Shell.Screenshot',
            '/org/gnome/Shell/Screenshot',
            'org.gnome.Shell.Screenshot',
            'Screenshot',
            new GLib.Variant('(bbs)', [false, false, filename]),
            new GLib.VariantType('(bs)'),
            Gio.DBusCallFlags.NONE,
            -1,
            null,
            (connection, result) => {
                try {
                    const [success, filenameUsed] =
                        connection.call_finish(result).deepUnpack();
                    invocation.return_value(
                        new GLib.Variant('(bsii)', [
                            success, filenameUsed, pointerX, pointerY,
                        ]));
                } catch (e) {
                    invocation.return_gerror(e);
                }
            });
    }
}
