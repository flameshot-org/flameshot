#!/usr/bin/env sh
set -eu

uuid="flameshot-native@flameshot.org"
script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
source_dir="${script_dir}/${uuid}"
target_dir="${HOME}/.local/share/gnome-shell/extensions/${uuid}"

mkdir -p "$(dirname -- "${target_dir}")"
rm -rf "${target_dir}"
cp -r "${source_dir}" "${target_dir}"

if command -v gnome-extensions >/dev/null 2>&1; then
    gnome-extensions enable "${uuid}" || true
fi

printf '%s\n' "Installed ${uuid} to ${target_dir}"
printf '%s\n' "Log out and back in if GNOME Shell has not loaded it yet."
