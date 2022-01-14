set -l SUBCOMMANDS gui screen full launcher config

####################
# HELPER FUNCTIONS #
####################

# Complete the subcommand provided as the first argument.
# The rest of the arguments are the same as fish's `complete`. The option
# '-c flameshot' is implicit.
function __flameshot_complete --argument-names cmd
     set -l conditions "__fish_seen_subcommand_from $cmd"
     argparse -i "n/condition=" -- $argv[2..]
     [ -n "$_flag_n" ] && set -l conditions "$conditions && $_flag_n"
     complete -c flameshot $argv -n "$conditions"
end

# Return success if the command line contains no positional arguments
function __flameshot_no_positional_args
    set -l -- args    (commandline -po)         # cmdline broken up into list
    set -l -- cmdline (commandline -p)          # single string
    set -l -- n       (count $args)             # number of cmdline tokens
    for i in (seq 2 $n)
        set -l arg $args[$i]
        [ -z "$arg" ] && continue               # can be caused by '--' argument

        # If the the last token is a positional argument and there is no
        # trailing space, we ignore it
        [ "$i" = "$n" ] && [ (string sub -s -1 "$cmdline") != ' ' ] && break

        if string match -rvq '^-' -- "$arg"     # doesn't start with -
            return 1
        end
    end
    # contains a '--' argument
    string match -r -- '\s--\s' "$cmdline" && return 1
    return 0
end

# Complete paths matching $argv
function __flameshot_complete_paths
    complete -C"nOnExIsTeNtCoMmAndZIOAGA2329jdbfaFkahDf21234h8z43 $argv"
end

# Complete the region option
function __flameshot_complete_region --argument subcommand
    if [ "$subcommand" = "screen" ]
        echo all\tCapture entire screen
    else
        echo all\tCapture all screens
        echo screen0\tCapture screen 0
        echo screen1\tCapture screen 1
        echo screen2\tCapture screen 2
        echo screen3\tCapture screen 3
    end
    echo WxH+X+Y\tCustom region in pixels
end

# Complete screen numbers
function __flameshot_complete_screen_number
    echo 0\tScreen 0
    echo 1\tScreen 1
    echo 2\tScreen 2
    echo 3\tScreen 3
end

###############
# COMPLETIONS #
###############

# No subcommand
complete -c flameshot -n __flameshot_no_positional_args -f -a "$SUBCOMMANDS"
complete -c flameshot -l "help"                 -s "h"  -f   -d "Display help message"
complete -c flameshot -l "version"              -s "v"  -f   -d "Display version information" -n __flameshot_no_positional_args

# GUI subcommand
__flameshot_complete gui                                -f
__flameshot_complete gui -l "path"              -s "p"  -rk  -d "Output file or directory"
__flameshot_complete gui -l "clipboard"         -s "c"  -f   -d "Copy screenshot to the clipboard"
__flameshot_complete gui -l "delay"             -s "d"  -frk -d "Delay time in milliseconds"
__flameshot_complete gui -l "region"                    -frk -d "Screenshot region to select (WxH+X+Y)" -a "(__flameshot_complete_region gui)"
__flameshot_complete gui -l "raw"               -s "r"  -f   -d "Print raw PNG capture"
__flameshot_complete gui -l "print-geometry"    -s "g"  -f   -d "Print geometry of the selection"
__flameshot_complete gui -l "upload"            -s "u"  -f   -d "Upload the screenshot"
__flameshot_complete gui -l "pin"                       -f   -d "Pin the screenshot to the screen"
__flameshot_complete gui -l "accept-on-select"  -s "s"  -f   -d "Accept capture as soon as a selection is made"

# SCREEN subcommand
__flameshot_complete screen                             -f
__flameshot_complete screen -l "number"         -s "n"  -frk -d "Screen number (starting from 0)" -a "(__flameshot_complete_screen_number)"
__flameshot_complete screen -l "path"           -s "p"  -rk  -d "Output file or directory"
__flameshot_complete screen -l "clipboard"      -s "c"  -f   -d "Copy screenshot to the clipboard"
__flameshot_complete screen -l "delay"          -s "d"  -frk -d "Delay time in milliseconds"
__flameshot_complete screen -l "region"                 -frk -d "Screenshot region to select (WxH+X+Y)" -a "(__flameshot_complete_region screen)"
__flameshot_complete screen -l "raw"            -s "r"  -f   -d "Print raw PNG capture"
__flameshot_complete screen -l "upload"         -s "u"  -f   -d "Upload the screenshot"
__flameshot_complete screen -l "pin"                    -f   -d "Pin the screenshot to the screen"

# FULL command
__flameshot_complete full                               -f
__flameshot_complete full   -l "path"           -s "p"  -rk  -d "Output file or directory"
__flameshot_complete full   -l "clipboard"      -s "c"  -f   -d "Copy screenshot to the clipboard"
__flameshot_complete full   -l "delay"          -s "d"  -frk -d "Delay time in milliseconds"
__flameshot_complete full   -l "region"                 -frk -d "Screenshot region to select (WxH+X+Y)" -a "(__flameshot_complete_region full)"
__flameshot_complete full   -l "raw"            -s "r"  -f   -d "Print raw PNG capture"
__flameshot_complete full   -l "upload"         -s "u"  -f   -d "Upload the screenshot"

# LAUNCHER command doesn't have any completions specific to itself

# CONFIG command -- TODO will be completed in a future version
__flameshot_complete config                             -f
__flameshot_complete config -l "check"                  -f   -d "Check the configuration for errors"
