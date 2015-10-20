#!/bin/bash

source ./options.sh $@

cleanupDirInNeeded $APP_DIR

cd $APP_DIR
echo "Building app"
QT_SELECT=qt5 $QMAKE "CONFIG-=debug_and_release" "CONFIG+=release" $SRC_DIR
make $JOBS

