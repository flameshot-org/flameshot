#compdef flameshot
# ------------------------------------------------------------------------------
# Description
# -----------
#
#  Completion script for the flameshot command line interface
#  (https://github.com/flameshot-org/flameshot).
#
# ------------------------------------------------------------------------------
# How to use
# -------
#
# Copy this file to /usr/share/zsh/site-functions/_flameshot
#


# gui

_flameshot_gui_opts=(
    {-p,--path}'[Existing directory or new file to save to]':dir:_files
    {-c,--clipboard}'[Save the capture to the clipboard]'
    {-d,--delay}'[Delay time in milliseconds]'
    "--region[Screenshot region to select <WxH+X+Y or string>]"
    "--last-region[Repeat screenshot with previously selected region]"
    {-r,--raw}'[Print raw PNG capture]'
    {-g,--print-geometry}'[Print geometry of the selection in the format WxH+X+Y. Does nothing if raw is specified]'
    {-u,--upload}'[Upload screenshot]'
    "--pin[Pin the capture to the screen]"
    {-s,--accept-on-select}'[Accept capture as soon as a selection is made]'
    {-h,--help}'[Show the available arguments]'
)

_flameshot_gui() {
    _arguments -s : \
    "$_flameshot_gui_opts[@]"
}


# screen

_flameshot_screen_opts=(
    {-n,--number}'[Define the screen to capture (starting from 0). Default: screen containing the cursor]'
    {-p,--path}'[Existing directory or new file to save to]':dir:_files
    {-c,--clipboard}'[Save the capture to the clipboard]'
    {-d,--delay}'[Delay time in milliseconds]'
    "--region[Screenshot region to select <WxH+X+Y or string>]"
    {-r,--raw}'[Print raw PNG capture]'
    {-u,--upload}'[Upload screenshot]'
    "--pin[Pin the capture to the screen]"
    {-h,--help}'[Show the available arguments]'
)

_flameshot_screen() {
    _arguments -s : \
    "$_flameshot_screen_opts[@]"
}


# full

_flameshot_full_opts=(
    {-p,--path}'[Existing directory or new file to save to]':dir:_files
    {-c,--clipboard}'[Save the capture to the clipboard]'
    {-d,--delay}'[Delay time in milliseconds]'
    "--region[Screenshot region to select <WxH+X+Y or string>]"
    {-r,--raw}'[Print raw PNG capture]'
    {-u,--upload}'[Upload screenshot]'
    {-h,--help}'[Show the available arguments]'
)

_flameshot_full() {
    _arguments -s : \
    "$_flameshot_full_opts[@]"
}


# config

_flameshot_config_opts=(
    {-a,--autostart}'[Enable or disable run at startup (true/false)]'
    {-f,--filename}'[Set the filename pattern]'
    {-n,--notification}'[Enable or disable the notification (true/false)]'
    {-t,--trayicon}'[Enable or disable the tray icon (true/false)]'
    {-s,--showhelp}'[Show the help message in the capture mode (true/false)]'
    {-m,--maincolor}'[Define the main UI color (hexadecimal)]'
    {-k,--contrastcolor}'[Define the contrast UI color (hexadecimal)]'
    "--check[Check the configuration for errors]"
    {-h,--help}'[Show the available arguments]'
)

_flameshot_config() {
    _arguments -s : \
    "$_flameshot_config_opts[@]"
}


# Main handle
_flameshot() {
    local curcontext="$curcontext" ret=1
    local -a state line commands

    commands=(
        "gui:Start the manual capture in GUI mode"
        "screen:Capture a single screen (one monitor)"
        "full:Capture the entire desktop (all monitors)"
        "launcher:Open the capture launcher"
        "config:Configure Flameshot"
    )

    _arguments -C -s -S -n \
        '(- 1 *)'{-v,--version}"[display version information]: :->full" \
        '(- 1 *)'{-h,--help}'[[display usage information]: :->full' \
        '1:cmd:->cmds' \
        '*:: :->args' && ret=0

    case "$state" in
        (cmds)
            _describe -t commands 'commands' commands
        ;;
        (args)
        local cmd
        cmd=$words[1]
        case "$cmd" in
            (gui)
                _flameshot_gui && ret=0
            ;;
            (screen)
                _flameshot_screen && ret=0
            ;;
            (full)
                _flameshot_full && ret=0
            ;;
            (config)
                _flameshot_config && ret=0
            ;;
            (*)
                _default && ret=0
            ;;
        esac
        ;;
        (*)
        ;;
    esac

    return ret
}

_flameshot

#
# Editor modelines  -  https://www.wireshark.org/tools/modelines.html
#
# Local variables:
# mode: sh
# c-basic-offset: 4
# tab-width: 4
# indent-tabs-mode: nil
# End:
#
# vi: set filetype=zsh shiftwidth=4 tabstop=4 expandtab:
# :indentSize=4:tabSize=4:noTabs=true:
#
