# How To Build Flameshot Release On The GitHub

Build new release with version: `X.Y.Z.F`.

_Note: `X`, `Y`, `Z` and `F` should be digits._

## Namecheap release procedure

- Build `pre-release`.
- Notify Namecheap Local-IT team that pre-release is ready for testing.
- Wait for testing by CS-Testing group (Local-IT team will notify maintainer).
- Publish `release` if no critical issues found.
- Skip release and fix issues if some critical issues found.

## Build new release

### Clone repository and create a release branch

```shell
git clone git@github.com:namecheap/flameshot.git
git checkout -b release/vX.Y.Z.F
```

### Update release version

#### Update version in CMakeLists.txt file
Update version in the file `./CMakeLists.txt`:
```makefile
set(FLAMESHOT_VERSION X.Y.Z.F)
```

#### Update version in flameshot.spec file
Update version in the file `./packaging/rpm/flameshot.spec`:
```ini
Version: X.Y.Z.F
```

#### Commit and set git tag with required version 
```shell
git commit -am "Release vX.Y.X.F"
git tag vX.Y.Z.F
```

### Push release branch with release tag
```shell
git push origin -u release/vX.Y.Z.F --tags
```

### Create pull request on the GitHub

Open Flameshot on the GitHub: https://github.com/namecheap/flameshot/pulls

Choose release branch `release/vX.Y.Z.F` and create pull request to the `master` branch of the Namecheap Flameshot repository (not to the upstream).


### Merge to the master

Check for a successful build at GitHub actions: https://github.com/namecheap/flameshot/actions)
Usually it takes around 30 minutes, sometimes it may take longer.

Merge `release/vX.Y.Z.F` to the `master` on success.

_Note: Check the log if there'll be some fails. Sometimes some third-party resources are not available (download issues), just re-run the job._


## Publish Pre-Release & Release

### Create pre-release on the GitHub

Create release on the GitHub for the required version: 
https://github.com/namecheap/flameshot/releases

Find release `vX.Y.Z.F` and click on it and then click on `Edit tag`. 

_Note: do not forget to set `pre-release` flag._


### Add binary files to release on the GitHub

Currently, CI/CD is not completely finished and still requires some manual actions.

- Download binary packages for all platforms for the latest release in the GitHub actions:
    - [Windows](https://github.com/namecheap/flameshot/actions/workflows/Windows-pack.yml),
      download `VS 2019 x86-installer` (win32) and `VS 2019 x64-installer` (win64) files.
      You can find download link in the `Upload Windows installer(daily build)` section.
    - [MacOS](https://github.com/namecheap/flameshot/actions/workflows/MacOS-pack.yml), 
      you can find download link in the `Upload dmg package` section.
    - [Linux](https://github.com/namecheap/flameshot/actions/workflows/Linux-pack.yml), 
      you can find download link in the `Upload <distro name> package (daily build)` for each distributive. 
- Upload all downloaded files from `GitHub actions` to the new release on the GitHub (use link from the previous section).

### Generate release notes

Generate release notes based on git log history
```shell
git log --pretty=oneline `git tag --sort=-committerdate | head -1`...`git tag --sort=-committerdate | head -2 | tail -1` |cut -d " " -f 2- |grep -v "Merge pull request" | sed 's/^/- /'
```

Copy output to the clipboard and paste on the release page.
Remove not significant lines and duplicates like a few translations fixes during one release etc.

### Publish pre-release

Set `pre-release` flag on the release page and press save.

### Publish release

- Open release page
- Press edit button
- Uncheck pre-release flag
- Press save
