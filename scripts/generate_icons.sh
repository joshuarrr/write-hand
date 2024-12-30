#!/bin/bash

# Create iconset directory
mkdir -p build/WriteHand.app/Contents/Resources/app.iconset

# Convert SVG to different size PNGs
for size in 16 32 64 128 256 512 1024; do
  # Normal resolution
  inkscape -w $size -h $size icons/app.svg -o "build/WriteHand.app/Contents/Resources/app.iconset/icon_${size}x${size}.png"

  # High resolution (2x) - required for Retina displays
  if [ $size -le 512 ]; then
    inkscape -w $((size * 2)) -h $((size * 2)) icons/app.svg -o "build/WriteHand.app/Contents/Resources/app.iconset/icon_${size}x${size}@2x.png"
  fi
done

# Generate icns file
iconutil -c icns build/WriteHand.app/Contents/Resources/app.iconset
