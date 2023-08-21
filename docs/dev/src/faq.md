
# FAQ

!!! todo

    Incomplete page.

### How do I create a new subcommand?

### How do I add a new tool?

### How do I add a new config setting?

There are currently two groups of settings: `General` and `Shortcuts`.
The necessary steps are usually the following:

- Determine a name for the setting - for a general setting, it must be a valid
  C++ identifier, for a shortcut it must be the name of a tool type from TODO.
- Add a getter and a setter for the setting in [`ConfigHandler`][ConfigHandler].
  For most settings you should use the
  [`CONFIG_GETTER_SETTER`][CONFIG_GETTER_SETTER] macro. If your setting is
  unusual enough you may need to use [`CONFIG_GETTER`][CONFIG_GETTER] or
  [`CONFIG_SETTER`][CONFIG_SETTER] individually, or even need to create the
  methods manually.
- If you need custom validation or conversion for the value, you must create a
  subclass of [`ValueHandler`][ValueHandler]. Otherwise you can use one of the
  existing ones in [valuehandler.h][].
- If you want to make your setting available in the configuration GUI (usually
  you do), you should add the appropriate widgets into one of the tabs of
  [`ConfigWindow`][ConfigWindow]. If your setting doesn't fit into any of the
  existing tabs, you can add a new one, but please discuss it with us first.

To get a deeper understanding of how the configuration works, please see
[Configuration][config].

### How do I add a new export action? (@borgmanJeremy @mehrad This is my preferred terminology over final action, need consensus)

[config]: index.md#configuration
[confighandler.h]: flameshot/confighandler_8h
[confighandler.cpp]: flameshot/confighandler_8cpp
[valuehandler.h]: flameshot/valuehandler_8h
[ValueHandler]: flameshot/classValueHandler
[ConfigHandler]: flameshot/classConfigHandler
[ConfigWindow]: flameshot/classConfigWindow
[CONFIG_GETTER_SETTER]: flameshot/confighandler_8h/#define-config_getter_setter
[CONFIG_GETTER]: flameshot/confighandler_8h/#define-config_getter
[CONFIG_SETTER]: flameshot/confighandler_8h/#define-config_setter
