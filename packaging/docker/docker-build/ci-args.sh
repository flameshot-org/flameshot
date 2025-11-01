# Optional: CI args for docker builds
# Set these in CI if you need to override tags or builder image names
export IMAGE_BUILDER_NAME=ghcr.io/your-org/flameshot-builder
export IMAGE_RUNTIME_NAME=ghcr.io/your-org/flameshot

# Example: use a commit-specific tag
# export IMAGE_TAG="commit-${GITHUB_SHA::8}"