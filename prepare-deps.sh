#!/bin/bash

set -Eeuo pipefail

# Setup paths where clickable will look for the sources
ROOT_DIR=$(git rev-parse --show-toplevel)
QZXING_SRC_DIR=$ROOT_DIR/libs/qzxing

# Remove old downloads
rm -rf $QZXING_SRC_DIR ${QZXING_SRC_DIR}_TMP

# Download sources

git clone https://github.com/ftylitak/qzxing.git "${QZXING_SRC_DIR}_TMP"
cd "${QZXING_SRC_DIR}_TMP"
git checkout 641da3618b3c3e386d32c70a208a49df72839c0a
git apply ../../qzxing.patch
cd ..
mv "${QZXING_SRC_DIR}_TMP/src" "${QZXING_SRC_DIR}"
rm -rf "${QZXING_SRC_DIR}_TMP"
cd "${QZXING_SRC_DIR}"
cp ../../QZXingConfig.cmake.in .
cd ..