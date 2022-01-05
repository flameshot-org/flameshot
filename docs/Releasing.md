# Checklist for making a new release

These are the code changes that need to take place
- [ ] Update version in CMakeLists.txt
- [ ] Update version and changelog at packaging/debian/changelog
- [ ] Update version and changelog at packaging/rpm/flameshot.spec
- [ ] Update data/appdata/flameshot.metainfo.xml
- [ ] Update MacOs version in cmake/modules/MacOSXBundleInfo.plist.in 
- [ ] Commit and push to a PR
- [ ] Merge PR to main
- [ ] Create and push git tag on main
- [ ] Manually retrigger the github actions so it uses the latest git tag 

These are the steps for actually making the release
- [ ] Download all binaries from CI run started from PR related to code changes shown above
- [ ] Create sha256 for each binary and compare against sha256 shown in the CI to verify there was no corruption or inserted malware.
- [ ] Create a new "New Release" in github and explain changes in release notes
- [ ] Upload all binaries and sha's
- [ ] Update flatpak manifest for flathub: https://github.com/flathub/org.flameshot.Flameshot
- [ ] Push snapcraft edge release to stable
- [ ] If this is a major release coordinate with sign path on signed windows binaries
- [ ] Update change log on [website](https://github.com/flameshot-org/flameshot-org.github.io/) data/changelog.md
- [ ] Update version on [website](https://github.com/flameshot-org/flameshot-org.github.io/blob/master/_coverpage.md)
