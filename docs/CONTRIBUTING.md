# Contributing

Contributions are welcome! Here's how you can help:

- [Translating](#translations)
- [Contributing code](#code)
- [Reporting issues](#issues)
- [Donating](#donations)

## Translations

See [translation instructions](https://github.com/flameshot-org/translation-instruction).

## Code

Ask question in the [Discussions](https://github.com/flameshot-org/flameshot/discussions) or issues and we will happily try to help as much as we can.

For small fixes or incremental improvements simply fork the repo and follow the process below. For larger changes submit an [RFC:](RFC.md)

1. [Fork](https://help.github.com/articles/fork-a-repo/) the repository and [clone](https://help.github.com/articles/cloning-a-repository/) your fork.

2. Start coding!
    - Implement your feature
    - Check your code works as expected
    - Run the code formatter: `clang-format -i $(git ls-files "*.cpp" "*.h")`

3. Commit your changes to a new branch (not `master`, one change per branch) and push it to your fork:
    - Commit messages should:
        - Header line: explain the commit in one line (use the imperative)
        - Be descriptive
        - Have a first line with less than *80 characters* and have a second line that is *empty* if you want to add a description.
    - make sure the Github Actions are successful after pushing to your local fork and before creating Pull Request.

4. Once you are happy with your changes, submit a pull request.
     - Open the pull-request
     - Add a short description explaining briefly what you've done (or if it's a work-in-progress - what you need to do)
     - Follow [AI-use disclosure](https://github.com/flameshot-org/flameshot/blob/master/AGENTS.md) if you have used AI in any shape or form in the development process.

## Issues

1. Do a quick search on GitHub to check if the issue has already been reported.
2. [Open an issue](https://github.com/flameshot-org/flameshot/issues/new) and describe the issue you are having - you could include:
     - Screenshots
     - Ways to reproduce the issue.
     - Your Flameshot version.
     - Your platform (e.g. Windows 10 or Ubuntu 24.04 x64)
     - Your monitor configurations (resolution, scaling, orientation, etc.)

After reporting you should aim to answer questions or clarifications as this helps pinpoint the cause of the issue.

## Donations

Please refer to [the donation page on website](https://flameshot.org/donate/) for details, but as a brief recap, Flameshot does **not** accept financial donations, but we encourage you to put code bounty on the platform of your choice on the issues you like to be addressed.
