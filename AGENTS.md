# Repository instructions

## Pull requests

Every pull request must include an AI-use disclosure.

The disclosure must classify the change as exactly one of:

- No AI used
- AI-assisted
- AI-generated

- **AI-assisted**: A human authored or substantially directed the change, but have used an AI system for e.g. completion, refactoring, tests, documentation, or debugging.
- **AI-generated**: An AI agent generated a substantial part of the patch or autonomously performed the implementation.

If AI-assisted or AI-generated, the PR message should include:

- Model or system name
- Provider or local deployment
- Model version, when known
- Tool or agent used
- Parts of the change generated or substantially modified by AI
- Human reviewer
- Tests and validation performed by human

Use the following section in the pull-request body:

## AI-use disclosure

- Classification: [ No AI used | AI-assisted | AI-generated]
- Model/system:
- Provider or local deployment:
- Version:
- Tool/agent:
- Scope of AI contribution:
- Human review: [Y|N]
- Tests and validation: [Y|N]

Do not claim that no AI was used merely because a human created the commit. The classification concerns how the code was produced, not Git author metadata.
