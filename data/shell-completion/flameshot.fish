
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

# define the subcommands
set -l SUBCOMMANDS gui screen full launcher config

# No subcommand
complete -c flameshot                                                                                        --no-files --arguments "$SUBCOMMANDS" --condition __flameshot_no_positional_args 
complete -c flameshot --long-option "help"    --short-option "h" --description "Display help message"        --no-files
complete -c flameshot --long-option "version" --short-option "v" --description "Display version information" --no-files                            --condition __flameshot_no_positional_args

# GUI subcommand
__flameshot_complete gui --no-files
__flameshot_complete gui --long-option "path"             --short-option "p" --description "Output file or directory"                          --require-parameter
__flameshot_complete gui --long-option "clipboard"        --short-option "c" --description "Copy screenshot to the clipboard"                                      --no-files
__flameshot_complete gui --long-option "delay"            --short-option "d" --description "Delay time in milliseconds"                        --require-parameter --no-files
__flameshot_complete gui --long-option "region"                              --description "Screenshot region to select (WxH+X+Y)"             --require-parameter            --arguments "(__flameshot_complete_region gui)"
__flameshot_complete gui --long-option "last-region"                         --description "Repeat screenshot with previously selected region"                     --no-files
__flameshot_complete gui --long-option "raw"              --short-option "r" --description "Print raw PNG capture"                                                 --no-files
__flameshot_complete gui --long-option "print-geometry"   --short-option "g" --description "Print geometry of the selection"                                       --no-files
__flameshot_complete gui --long-option "upload"           --short-option "u" --description "Upload the screenshot"                                                 --no-files
__flameshot_complete gui --long-option "pin"                                 --description "Pin the screenshot to the screen"                                      --no-files
__flameshot_complete gui --long-option "accept-on-select" --short-option "s" --description "Accept capture as soon as a selection is made"                         --no-files
__flameshot_complete gui --long-option "help"             --short-option "h" --description "Show the available arguments"                                          --no-files

# SCREEN subcommand
__flameshot_complete screen --no-files
__flameshot_complete screen --long-option "number"      --short-option "n" --description "Screen number (starting from 0)"                    --require-parameter --no-files --arguments "(__flameshot_complete_screen_number)"
__flameshot_complete screen --long-option "path"        --short-option "p" --description "Output file or directory"                           --require-parameter
__flameshot_complete screen --long-option "clipboard"   --short-option "c" --description "Copy screenshot to the clipboard"                                       --no-files
__flameshot_complete screen --long-option "delay"       --short-option "d" --description "Delay time in milliseconds"                         --require-parameter --no-files
__flameshot_complete screen --long-option "region"                          --description "Screenshot region to select (WxH+X+Y)"             --require-parameter --no-files --arguments "(__flameshot_complete_region screen)"
__flameshot_complete screen --long-option "raw"         --short-option "r" --description "Print raw PNG capture"                                                 --no-files
__flameshot_complete screen --long-option "upload"      --short-option "u" --description "Upload the screenshot"                                                 --no-files
__flameshot_complete screen --long-option "pin"                            --description "Pin the screenshot to the screen"                                      --no-files
__flameshot_complete screen --long-option "help"        --short-option "h" --description "Show the available arguments"                                          --no-files

# FULL command
__flameshot_complete full --no-files
__flameshot_complete full --long-option "path"        --short-option "p" --description "Output file or directory"                          --require-parameter
__flameshot_complete full --long-option "clipboard"   --short-option "c" --description "Copy screenshot to the clipboard"                                      --no-files
__flameshot_complete full --long-option "delay"       --short-option "d" --description "Delay time in milliseconds"                        --require-parameter --no-files
__flameshot_complete full --long-option "region"                         --description "Screenshot region to select (WxH+X+Y)"             --require-parameter --no-files --arguments "(__flameshot_complete_region full)" --keep-order
__flameshot_complete full --long-option "raw"         --short-option "r" --description "Print raw PNG capture"                                                 --no-files
__flameshot_complete full --long-option "upload"      --short-option "u" --description "Upload the screenshot"                                                 --no-files
__flameshot_complete full --long-option "help"        --short-option "h" --description "Show the available arguments"                                          --no-files

# LAUNCHER command
__flameshot_complete launcher --no-files

# CONFIG command
__flameshot_complete config --no-files
__flameshot_complete config --long-option "autostart"      --short-option "a" --description "Enable or disable run at startup"           --require-parameter --no-files --arguments "true false"
__flameshot_complete config --long-option "filename"       --short-option "f" --description "Set the filename pattern"                   --require-parameter --no-files 
__flameshot_complete config --long-option "notification"   --short-option "n" --description "Enable or disable the notification"         --require-parameter --no-files --arguments "true false"
__flameshot_complete config --long-option "trayicon"       --short-option "t" --description "Enable or disable the tray icon"            --require-parameter --no-files --arguments "true false"
__flameshot_complete config --long-option "showhelp"       --short-option "s" --description "Show the help message in the capture mode"  --require-parameter --no-files --arguments "true false"
__flameshot_complete config --long-option "maincolor"      --short-option "m" --description "Define the main UI color (hexadecimal)"     --require-parameter --no-files 
__flameshot_complete config --long-option "contrastcolor"  --short-option "k" --description "Define the contrast UI color (hexadecimal)" --require-parameter --no-files 
__flameshot_complete config --long-option "check"                             --description "Check the configuration for errors"                             --no-files
__flameshot_complete config --long-option "help"           --short-option "h" --description "Show the available arguments"                                   --no-files
