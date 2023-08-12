# Maintaining the documentation

The narrative documentation is written in markdown and built into HTML using
[MkDocs][mkdocs], particularly the [MkDocs material theme][mkdocs-material]. The
source code documentation is generated using Doxygen and adapted for MkDocs
using [MkDoxy][mkdoxy] (a tweaked custom fork of the original). The source code
of this documentation can be found [here][doc-source].

!!! tip

    In order to edit a page from the documentation, click the :material-pencil:
    button in the top right corner of the page. Please note that this button
    won't work within the API section of the documentation - the button is
    removed from there during [post-processing][], but it will still be visible
    when [serving the website locally][serving-locally].

## Serving locally
To serve the documentation locally, run the `make serve` target in the
`docs/dev` directory. The server is available at the port designated in the
output of the command.

## Notes and conventions
- When you add new files or rename existing files or section names, be sure to
  edit the `nav` property of [mkdocs.yml][mkdocs.yml].
- Always insert links as [reference style
  links][markdown:reference-style-links]. This will make the docs source code
  more readable and make broken links more easily detectable and replaceable.

### Post-processing
There are some tweaks we make to the generated HTML documentation. We do that in
the `make build` target, by running the [post-process.sh][post-process.sh]
script. To see what post-processing we do, see that file.

For this reason, the version of the documentation served locally using `make
serve` will not match the generated HTML documentation 100%. But those
inconsistencies are few and minor.

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

The official website itself is served from this [repo][website-repo]. That repo
contains the user documentation. It's deployed using GitHub pages -- the served
files can be found on the [gh-pages][] branch of that repo. This branch is
automatically created by the [build][website-build] workflow on master.

To make the developer docs available on the official site, we use a custom
GitHub action called [deploy-dev-docs][] in the [flameshot][] repo. This action
will build and deploy this documentation into the `docs/dev` subdirectory of the
[gh-pages][] branch.

### The deploy-dev-docs GitHub workflow

This workflow checks out the flameshot website [repo][website-repo] and does the following:

- Creates a clean [dev-docs-staging][] branch (we'll explain why, below).
- Generates the developer docs under `docs/dev` there and makes a commit
- Checks out the [gh-pages][] branch and makes the same commit there
- Force-pushes the dev-docs-staging branch to the website repo
- Pushes the gh-pages branch to the website repo

Since the [gh-pages][] branch is re-created from scratch by the [website
deployment workflow][website-build], the commit that added the developer
documentation will be lost. That's why we have to re-apply the commit during the
[website deployment workflow][website-build] as well. **That is the reason why
we created the** [**dev-docs-staging**][dev-docs-staging] **branch in the first
place.**

!!! note

    The deploy-dev-docs workflow is set to run on the `docs` branch as well as `master`.
    This branch is used for debugging the developer docs and its associated
    workflows, without polluting the `master` branch with unnecessary commits.

#### Access tokens
In order to make changes to the [website repo][website-repo] from within a
workflow in the [flameshot repo][flameshot], the workflow needs to use an access
token, which it obtains from the `TOKEN_PUSH_TO_WEBSITE_REPO` secret.

The following process was used to set it up:

1. A flameshot organization member with write access must create a personal
   access token (PAT) [here][PAT] with write access to the [website repo][website-repo].
2. A secret named `TOKEN_PUSH_TO_WEBSITE_REPO` must be added to the
   [flameshot][] repo and its value must be set to the PAT. This can be done
   [here][action-secrets].

For best security practice, the token should be set to expire after some time
(currently ~6 months). The token can be regenerated without the need to recreate
it. After regeneration you will need to update the `TOKEN_PUSH_TO_WEBSITE_REPO`
secret, which can be done [here][edit-secret].

!!! tip

    The currently active PAT is owned by [veracioux][] and is set to expire on July
    31 2024. If you notice the token has expired, ask him to re-generate it.

<!-- Internal links -->
[post-processing]: #post-processing
[serving-locally]: #serving-locally

<!-- Flameshot-related pages -->
[flameshot]: https://github.com/flameshot-org/flameshot
[website]: https://flameshot.org
[doc-source]: https://github.com/flameshot-org/flameshot/tree/master/docs/dev
[website-repo]: https://github.com/flameshot-org/flameshot-org.github.io
[gh-pages]: https://github.com/flameshot-org/flameshot-org.github.io/tree/gh-pages
[dev-docs-staging]: https://github.com/flameshot-org/flameshot-org.github.io/tree/dev-docs-staging
[action-secrets]: https://github.com/flameshot-org/flameshot/settings/secrets/actions
[edit-secret]: https://github.com/flameshot-org/flameshot/settings/secrets/actions/TOKEN_PUSH_TO_WEBSITE_REPO

<!-- Files in flameshot repo -->
[mkdocs.yml]: https://github.com/flameshot-org/flameshot/blob/master/docs/dev/mkdocs.yml
[post-process.sh]: https://github.com/flameshot-org/flameshot/blob/master/docs/dev/post-process.sh
[deploy-dev-docs]: https://github.com/flameshot-org/flameshot/blob/master/.github/workflows/deploy-dev-docs.yml

<!-- Files in flameshot website repo -->
[website-build]: https://github.com/flameshot-org/flameshot-org.github.io/blob/master/.github/workflows/build.yml

<!-- External pages -->
[markdown:reference-style-links]: https://www.markdownguide.org/basic-syntax/#reference-style-links
[mkdocs]: https://www.mkdocs.org/
[mkdocs-material]: https://squidfunk.github.io/mkdocs-material
[mkdoxy-original]: https://github.com/JakubAndrysek/mkdoxy
[mkdoxy]: https://github.com/veracioux/mkdoxy
[PAT]: https://github.com/settings/tokens?type=beta
[veracioux]: https://github.com/veracioux
