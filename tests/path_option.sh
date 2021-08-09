#!/usr/bin/env sh

pkill flameshot
flameshot &

sleep 4

# Create a directory for file output
rm -rf _out 2>/dev/null
mkdir -p _out
cd _out

# NOTE THIS
# The dir _out will be have as the home directory
HOME="$PWD"
mkdir -p subdir

echo Nonexistent directory is not a valid path
flameshot screen -p ~/blah/blah
echo Saved relative to PWD
flameshot screen -p relative.png
echo Absolute paths work too
flameshot screen -p ~/absolute.png
echo Redundancy in the path will be removed
flameshot screen -p ~/subdir/..///redundancy_removed.png
echo If the destionation is a directory, the file name is generated from strf from the config
flameshot screen -p ./
echo If the output file has no suffix, it will be added
flameshot screen -p ~/without_suffix
echo If the suffix does not match the output format, it will be overwritten
flameshot screen -p ~/wrong_suffix.jpg
echo If the destination path exists, it will have _NUM appended to the base name
flameshot screen -p ~/absolute.png
echo We will do that again but without a suffix
flameshot screen -p ~/absolute

sleep 3

pkill flameshot
