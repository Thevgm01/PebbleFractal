#!/bin/bash

# Requires jq, ImageMagick

PLATFORMS=($(jq -r '.pebble.targetPlatforms[]' package.json))
DELAY=5

echo "Remaking media/gifs folder..."
rm -rf "media/gifs"
mkdir -p "media/gifs"
sleep 1

for PLATFORM in "${PLATFORMS[@]}"; do
  echo "Making gif for $PLATFORM with a $DELAY centisecond delay..."
  # A delay of 0.03 seconds between images is approximately 30FPS
  convert -delay "$DELAY" -loop 0 "media/images/$PLATFORM/*.png" "media/gifs/$PLATFORM.gif"
  echo "Saved as media/gifs/$PLATFORM.gif"
  sleep 1
done

echo "Done!"
