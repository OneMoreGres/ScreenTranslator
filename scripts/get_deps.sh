#!/bin/bash

source ./options.sh $@

echo "Downloading dependencies"
wget -O $DOWNLOAD_DIR/leptonica.tar.gz http://www.leptonica.com/source/leptonica-1.72.tar.gz
wget -O $DOWNLOAD_DIR/tesseract.tar.gz https://github.com/tesseract-ocr/tesseract/archive/3.04.00.tar.gz

