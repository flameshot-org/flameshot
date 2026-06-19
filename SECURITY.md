# Flameshot Security Policy

## Supported Versions

Only the **latest stable release** of Flameshot, or the **HEAD of the master branch**, is actively supported with security updates. If you discover a vulnerability, please verify if it persists in the current version before reporting.

## Reporting a Vulnerability

We take the security of our software seriously. If you believe you have found a security vulnerability in Flameshot, please report it to the development team responsibly. Please avoid framing a non-security bug as a security bug.

To report a vulnerability, please use [our GitHub security section](https://github.com/flameshot-org/flameshot/security "URL to the security page") or email admin@flameshot.org with the necessary details. Do not open a public GitHub issue for security disclosures.

If you are not sure whether something is a security flaw or vulnerability, send an email to admin@flameshot.org, and we will do our best to figure it out together.

## Report Requirements

To help us triage and resolve the issue efficiently, your report must include the following:

1. CVSS Vector File: A JSON file generated using the latest CVSS calculator from the [National Vulnerability Database](https://nvd.nist.gov/vuln-metrics/cvss "URL to NIST website that contains CVSS calculators")
2. Proof of Concept: Functional code or script that demonstrates the exploit
3. Reproducible Instructions: Clear, step-by-step documentation for replicating the vulnerability
4. If you have used any software or AI to detect the vulnerability, please disclose that information transparently

**Optional Information**:

* Potential Solution: Suggestions, patches, or code fixes to address the issue are highly appreciated.

# Out-of-Scope Vulnerabilities
To protect our volunteers' time, the following types of reports are considered out of scope unless they present a novel, unique threat vector:

- Vulnerabilities that require prior root/administrator access or an already compromised operating system
  - this is because if root is compromised, the user have bigger problem on their hand and the bad actor can do much worse than what is remotely feasible with a screenshot tool
- Theoretical issues or raw outputs from automated security scanners without a verified, functional Proof of Concept.

## Response Process

Upon receiving the report, the following will happen on our side:

1. **Acknowledgment**: If sent via email, we will acknowledge receipt of your report; otherwise, it is already documented in the GitHub repo.
2. **Triage**: The development team will validate the finding using your reproduction steps.
3. **Fix & Advisory**: If verified, we will work on a mitigation and coordinate a release date for the fix alongside a public security advisory.

## Our Responsibilities and Commitments

In this project, we consider ourselves responsible and committed to the following:

- Transparency (when appropriate and applicable)
- Evaluation and validation of reported potential vulnerabilities
- Take action regarding the reports as quickly as possible

All these should be interpreted in the context of volunteer-based Free and Libre Open Source Software. Also note that because the project is maintained by volunteers in their spare time, triage and patching may take longer than commercial software standards.

## Reporters’ Responsibilities

We believe reporters also have responsibilities:

- Provide accurate and detailed enough information when reporting (no misleading information or tricking the system)
- Honesty and avoiding exaggeration or downplaying
- Maintain civil and professional etiquette throughout the process.

