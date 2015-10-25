#!/bin/bash

source ./options.sh $@

cleanupDirInNeeded $APP_DIR

cd $APP_DIR
echo "Building app"
lrelease $SRC_DIR/ScreenTranslator.pro
$QMAKE -qt=qt5 "CONFIG-=debug_and_release" "CONFIG+=release" $SRC_DIR
make $JOBS

