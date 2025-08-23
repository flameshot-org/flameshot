#compdef flameshot

# Shell completion for flameshot command
# To be installed in "/usr/share/bash-completion/completions/flameshot"
# and "/usr/share/zsh/site-functions/"


_flameshot() {
	local prev cur cmd gui_opts full_opts config_opts
	COMPREPLY=()

	prev="${COMP_WORDS[COMP_CWORD-1]}"
	cur="${COMP_WORDS[COMP_CWORD]}"
	cmd="gui full config launcher screen"
	screen_opts="--number -n --path -p --clipboard -c --delay -d --region --raw -r --upload -u --pin --help"
	gui_opts="--path -p --clipboard -c --delay -d --region --last-region --raw -r --print-geometry -g --upload -u --pin --accept-on-select -s --help"
	full_opts="--path -p --clipboard -c --delay -d --region --raw -r --upload -u --help"
	config_opts="--autostart -a --filename -f --notifications -n --trayicon -t --showhelp -s --maincolor -m --contrastcolor -k --check"

	case "${prev}" in
		launcher)
			return 0
			;;
		screen)
			COMPREPLY=( $(compgen -W "$screen_opts --help -h" -- "${cur}") )
			return 0
			;;
		gui)
			COMPREPLY=( $(compgen -W "$gui_opts --help -h" -- "${cur}") )
			return 0
			;;
		full)
			COMPREPLY=( $(compgen -W "$full_opts --help -h" -- "${cur}") )
			return 0
			;;
		config)
			COMPREPLY=( $(compgen -W "$config_opts --help -h" -- "${cur}") )
			return 0
			;;
		-f|--filename|-p|--path)
			_filedir -d
			return 0
			;;
        # TODO: We should see how we can add the -n (for `config --notifications`) here that does not conflict with -n (for `screen --number`)
		-a|--autostart|-s|--showhelp|-t|--trayicon|--notifications)
			COMPREPLY=( $(compgen -W "true false" -- "${cur}") )
			return 0
			;;
		-d|--delay|-h|--help|-c|--clipboard|--version|-v|--number|-n)
			return 0
			;;
		*)
		;;
	esac

	# Options
	case "${cur}" in
		-*)
			COMPREPLY=( $( compgen -W "--version --help -v -h" -- "${cur}") )
			return 0
			;;
		--*)
			COMPREPLY=( $( compgen -W "--version --help" -- "${cur}") )
			return 0
			;;
		*)
			COMPREPLY=( $( compgen -W "${cmd}" -- "${cur}") )
			return 0
			;;
	esac
}

if [[ -n ${ZSH_VERSION} ]]; then
	autoload -U bashcompinit
	bashcompinit
fi

complete -F _flameshot flameshot
