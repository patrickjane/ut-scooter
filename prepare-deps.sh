#!/bin/bash

set -Eeuo pipefail

# Setup paths where clickable will look for the sources
ROOT_DIR=$(git rev-parse --show-toplevel)
MAPBOX_GL_NATIVE_SRC_DIR=$ROOT_DIR/libs/mapbox-gl-native
MAPBOX_GL_QML_SRC_DIR=$ROOT_DIR/libs/mapbox-gl-qml
S2GEOMETRY_SRC_DIR=$ROOT_DIR/libs/s2geometry

# Remove old downloads
rm -rf $MAPBOX_GL_NATIVE_SRC_DIR $MAPBOX_GL_QML_SRC_DIR $S2GEOMETRY_SRC_DIR

# Download sources
git clone -b mapbox-update-200607 https://github.com/rinigus/mapbox-gl-native.git $MAPBOX_GL_NATIVE_SRC_DIR --recurse-submodules  --shallow-submodules
git clone -b 1.7.5 https://github.com/rinigus/mapbox-gl-qml.git $MAPBOX_GL_QML_SRC_DIR
git clone https://github.com/rinigus/s2geometry.git $S2GEOMETRY_SRC_DIR
