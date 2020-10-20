# Checklist for making a new release

These are the code changes that need to take place
- [ ] Create and push git tag
- [ ] Update version in CMakeLists.txt
- [ ] Update version and changelog at data/debian/changelog
- [ ] Update version and changelog at data/rpm/flameshot.spec
- [ ] Update docs/appdata/flameshot.metainfo.xml

These are the steps for actually making the release
- [ ] Download all binaries from CI run started from PR related to code changes shown above
- [ ] Create sha256 for each binary and compare against sha256 shown in the CI to verify there was no corruption or inserted malware.
- [ ] Create a new "New Release" in githhub and explain changes in release notes
- [ ] Upload all binaries and sha's
- [ ] Update flatpak manifest for flathub: https://github.com/flathub/org.flameshot.Flameshot 
- [ ] Push snapcraft edge release to stable
- [ ] If this is a major release coordinate with sign path on signed windows binaries 
- [ ] Update change log on [website](https://github.com/flameshot-org/flameshot-org.github.io/) data/changelog.md
- [ ] Update version on [website](https://github.com/flameshot-org/flameshot-org.github.io/blob/master/_coverpage.md)

