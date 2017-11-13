 What to do in every release?

 - Check build with QT 5.3 .
 - Update translations.
 - Increase version number in `flameshot.pro`, the variable VERSION must be a version number in the format X.X.X, after the release, the next commit have to change the version to X.X.X-dev.
 - Properly generate the .deb file.
 - Releases always use annotated tags as in `git tag -a -m "v0.5.0"`
 - Add a changelog description in the Github's release.
