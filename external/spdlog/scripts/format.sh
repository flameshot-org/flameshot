#!/bin/bash

cd "$(dirname "$0")"/..
pwd
echo -n "Running dos2unix     "
find . -name "*\.h" -o -name "*\.cpp"|grep -v bundled|xargs -I {} sh -c "dos2unix '{}' 2>/dev/null; echo -n '.'"
echo
echo -n "Running clang-format "
find . -name "*\.h" -o -name "*\.cpp"|grep -v bundled|xargs -I {} sh -c "clang-format -i {}; echo -n '.'"
echo
echo -n "Running cmake-format "
find . -name "CMakeLists.txt" -o -name "*\.cmake"|grep -v bundled|xargs -I {} sh -c "cmake-format --line-width 120 --tab-size 4 --max-subgroups-hwrap 4 -i {}; echo -n '.'"
echo



