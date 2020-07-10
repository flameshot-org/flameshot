# Build Windows Installation:

## Prepare new release

### Step 1

Update translations

```
lupdate -recursive ./ -ts ./translations/*
```

### Step 2

Example update from `0.7.2` to `0.7.3`.

Update lines in the file `update_version_everywhere.sh`

```
BASE_VERSION_NEW=0.7.2
```

with:

```
BASE_VERSION_NEW=0.7.3
```

### Step 3

Run script `update_version_everywhere.sh`. 


### Step 4

Upload binaries from `appveyour` to github

### What does script `update_version_everywhere.sh` do:

* It will update version information in all project files (*.pro, *.cpp, ci/cd files etc).
* Create new commit with the text "Update version to 0.7.3" (according to current example)
* Push current branch into the repository.
* Add tag version `v0.7.3`


## Cancel release

You need to remove version tag, example:

```
git tag -d v0.7.3
git push origin :refs/tags/v0.7.3
```

## How to build a specific version

Example for version `0.7.3`.

Get required code and trigger a build:
```
git clone git@github.com:namecheap/flameshot.git
cd flameshot/
git checkout tags/v0.7.3 -b build/v0.7.3
git push origin -u build/v0.7.3
```

Appveyor CI will be triggered automatically on any push.

Open the link https://ci.appveyor.com/project/namecheap/flameshot in a browser and wait for a new build.

_Note: you can get currently install version from Windows Software center (Flameshot is included into approved NameCheaps Software list) or just as for someone from `Local IT` department._


# Old release documentation
What to do before every release?

 - Update translations.
 - Update travis version
 - Releases always use annotated tags as in `git tag -a v0.5.1 -m "version 0.5.1"`
 - Add a changelog description in the Github's release.
 - Update .pro hardcoded version.
 - Run cppcheck --enable=all 2> err.txt
 - Run codespell
