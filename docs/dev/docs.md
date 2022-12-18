# Maintaining the documentation

The narrative documentation is written in markdown and built into HTML using
[MkDocs][mkdocs], particularly the [MkDocs material theme][mkdocs-material]. The
source code documentation is generated using Doxygen and adapted for MkDocs
using [MkDoxy][mkdoxy] (a tweaked custom fork of the original). The source code
of this documentation can be found [here][doc-source].

!!! tip

    In order to edit a page from the documentation, click the :material-pencil:
    button in the top right corner of the page.

## Notes and conventions
- When you add new files or rename existing files or section names, be sure to
  edit the `nav` property of [mkdocs.yml][mkdocs.yml].
- Always insert links as [reference style
  links][markdown:reference-style-links]. This will make the docs source code
  more readable and make broken links more easily detectable and replaceable.

## Dependencies
```shell
pip install \
    mkdocs \
    mkdocs-material \
    git+https://github.com/veracioux/mkdoxy@v1.0.0
```

!!! note

    We use a forked version of [mkdoxy][mkdoxy-original] that can be found
    [here][mkdoxy], that fixes some annoying things from the original.

## Deployment

The developer documentation is served from the official Flameshot website
[flameshot.org][website]. Here's how.

The official website itself is served from this [repo](website-repo). That repo
contains the user documentation. It's deployed using GitHub pages -- the served
files can be found on the [gh-pages][] branch of that repo.

To make the developer docs available on the official site, we use a custom
GitHub action in the flameshot repo to build and deploy this documentation into
the `docs/dev` subdirectory of the [gh-pages][] branch.

!!! todo

    Create the action and link to it above :D

!!! todo

    Mention the post-process steps

[website]: https://flameshot.org
[doc-source]: https://github.com/flameshot-org/flameshot/tree/master/docs
[website-repo]: https://github.com/flameshot-org/flameshot-org.github.io
[gh-pages]: https://github.com/flameshot-org/flameshot-org.github.io/tree/gh-pages

[mkdocs]: https://www.mkdocs.org/
[mkdocs-material]: https://squidfunk.github.io/mkdocs-material
[mkdoxy-original]: https://github.com/JakubAndrysek/mkdoxy
[mkdoxy]: https://github.com/veracioux/mkdoxy
[mkdocs.yml]: https://github.com/flameshot-org/flameshot/docs/mkdocs.yml

[markdown:reference-style-links]: https://www.markdownguide.org/basic-syntax/#reference-style-links
