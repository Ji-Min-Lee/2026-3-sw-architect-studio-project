#!/bin/sh
# Configure git to use the shared hooks in .githooks/
# Run once after cloning: sh scripts/setup-hooks.sh

git config core.hooksPath .githooks
chmod +x .githooks/pre-commit
echo "Git hooks configured. Pre-commit test hook is now active."
