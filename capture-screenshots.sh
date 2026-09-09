#!/bin/bash

# Requires jq, ImageMagick

PLATFORMS=($(jq -r '.pebble.targetPlatforms[]' package.json))
FRAMES=60
INTERVAL=3

echo "Deleting media folder..."
rm -rf "media/"
mkdir -p "media/gifs"
sleep 1

echo "Building..."
CFLAGS="-DSCREENSHOTMODE" pebble build
if [ $? -ne 0 ]; then
  echo "Build failed, aborting"
  exit 1
fi

for PLATFORM in "${PLATFORMS[@]}"; do
  echo "Starting emulator for $PLATFORM..."
  mkdir -p "media/images/$PLATFORM"

  pebble install --emulator $PLATFORM
  if [ $? -ne 0 ]; then
    echo "Failed to install, skipping..."
    continue
  fi
  
  echo "Waiting for the fractal to settle..."
  
  coproc LOGS (pebble logs)
  while read -r line <&${LOGS[0]}; do
    FRAME=$(echo "$line" | grep -o 'SCREENSHOTFRAME:[0-9]*' | grep -o '[0-9]*')
    if [ -n "$FRAME" ]; then
      pebble screenshot "media/images/$PLATFORM/$FRAME.png"
      if [ "$FRAME" -ge $(($FRAMES - 1)) ]; then
        kill $LOGS_PID
        break
      fi
    fi
  done
  
  pebble kill
  sleep 1
  
  echo "Closing emulator..."
  sleep 1
  
  echo "Making gif..."
  # A delay of 0.03 seconds between images is approximately 30FPS
  convert -delay 5 -loop 0 "media/images/$PLATFORM/*.png" "media/gifs/$PLATFORM.gif"
  echo "Saved as media/$PLATFORM.gif"
  sleep 1
done

echo "Done!"
