#!/bin/bash

source ./options.sh $@

cleanupDirInNeeded $APP_DIR

cd $APP_DIR
echo "Building app"
$QT_LRELEASE $QT_CHOOSER $SRC_DIR/ScreenTranslator.pro
$QMAKE $QT_CHOOSER "CONFIG-=debug_and_release" "CONFIG+=release" $SRC_DIR
make $JOBS

