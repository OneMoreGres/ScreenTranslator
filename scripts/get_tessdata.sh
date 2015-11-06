#!/bin/bash

source ./options.sh $@

echo "Downloading and extracting tessdata"
wget -O $DOWNLOAD_DIR/tessdata.tar.gz https://github.com/tesseract-ocr/tessdata/archive/3.04.00.tar.gz
EXTRACT_DIR=$DOWNLOAD_DIR/tessdata
CLEAN=true
cleanupDirInNeeded $EXTRACT_DIR
tar zxf $DOWNLOAD_DIR/tessdata.tar.gz -C $EXTRACT_DIR
EXTRACTED=`ls $EXTRACT_DIR | grep tess`
mv $EXTRACT_DIR/$EXTRACTED/* $EXTRACT_DIR
rm -r $EXTRACT_DIR/$EXTRACTED

