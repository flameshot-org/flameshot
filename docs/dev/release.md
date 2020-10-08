 What to do before every release?

 - Update translations.
 - Update travis version
 - Releases always use annotated tags as in `git tag -a v0.5.1 -m "version 0.5.1"`
 - Add a changelog description in the Github's release.
 - Update .pro hardcoded version.
 - Run cppcheck --enable=all 2> err.txt
 - Run codespell
 