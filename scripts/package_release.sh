#!/bin/bash
# package_release.sh — Package UltraGlitch BitFucker release zip
#
# Usage:
#   ./scripts/package_release.sh
#
# Produces: v0.4.0-beta.zip in the project root
# Contents: Full source tree + dist/ compiled binaries (excludes build dirs and .git)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
VERSION=$(cat "$PROJECT_DIR/VERSION" | tr -d '[:space:]')
ZIP_NAME="${VERSION}.zip"

echo "=== Packaging UltraGlitch BitFucker ${VERSION} ==="
echo "Project dir: $PROJECT_DIR"
echo "Output:      $PROJECT_DIR/$ZIP_NAME"

cd "$PROJECT_DIR"

# Verify dist/ exists and has content
if [ ! -d "dist" ]; then
    echo "⚠️  Warning: dist/ directory not found. Zip will contain source only."
fi

# Create zip excluding build artifacts, .git, and large intermediate files
zip -r "$ZIP_NAME" . \
    -x ".git/*" \
    -x "build/*" \
    -x "build-win/*" \
    -x "build-mac/*" \
    -x ".DS_Store" \
    -x "*.zip" \
    -x "_deps/*"

echo ""
echo "=== Package contents ==="
unzip -l "$ZIP_NAME" | tail -5

echo ""
echo "🎉 Release packaged: $ZIP_NAME"
ls -lh "$ZIP_NAME"
