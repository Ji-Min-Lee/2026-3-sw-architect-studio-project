#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
OUTPUT_DIR="$PROJECT_DIR/build-rpi"

echo "=== TimeGrapher RPi Build (arm64) ==="

# Build Docker image
docker build \
  --platform linux/arm64 \
  -f "$PROJECT_DIR/Dockerfile.rpi" \
  -t timegrapher-rpi:latest \
  "$PROJECT_DIR"

# Extract binary
mkdir -p "$OUTPUT_DIR"
docker create --name tg-extract --platform linux/arm64 timegrapher-rpi:latest
docker cp tg-extract:/build/TimeGrapher "$OUTPUT_DIR/TimeGrapher"
docker rm tg-extract

echo ""
echo "=== Build complete ==="
echo "Binary: $OUTPUT_DIR/TimeGrapher"
file "$OUTPUT_DIR/TimeGrapher"

# Deploy to RPi (optional: set RPI_HOST env var)
if [ -n "$RPI_HOST" ]; then
  echo ""
  echo "=== Deploying to $RPI_HOST ==="
  scp "$OUTPUT_DIR/TimeGrapher" "$RPI_HOST:~/TimeGrapher"
  echo "Done. Run: ssh $RPI_HOST '~/TimeGrapher'"
fi
