#!/bin/bash

IMAGE="esp8266-clean"
PROJECT_NAME="EcoLogic_manager"

LIB_PATH="$(pwd)/project_libraries:/opt/project_libs"
SKETCH_PATH="$(pwd)/..:/workspace"

# Copy libraries if not exist
if [ ! -d "project_libraries" ]; then
  echo "Copying libraries..."
  if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    cp -r /c/MyDocuments/Programming/Projects_Arduino/libraries project_libraries
  else
    cp -r ~/Arduino/libraries project_libraries
  fi
fi

echo "Starting compilation..."

MSYS_NO_PATHCONV=1 docker run --rm \
  -v "$LIB_PATH" \
  -v "$SKETCH_PATH" \
  $IMAGE \
  arduino-cli compile \
  --fqbn esp8266:esp8266:generic:eesz=4M1M \
  --libraries /opt/project_libs \
  --build-path /workspace/$PROJECT_NAME/build \
  --verbose \
  /workspace/$PROJECT_NAME

if [ $? -eq 0 ]; then
  echo "Compilation successful. Binary: ../$PROJECT_NAME/build/$PROJECT_NAME.ino.bin"
else
  echo "Compilation error. Check logs above."
  exit 1
fi
