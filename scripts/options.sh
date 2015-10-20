#!/bin/bash

set -e

PLATFORM=linux
CLEAN=false
JOBS=""
QMAKE=qmake
QT_LIB_DIR=/usr/lib/x86_64-linux-gnu
STRIP=strip

for arg in ${@}; do
  case "$arg" in
    "linux" ) PLATFORM=linux;;
    "mingw" ) PLATFORM=mingw;;
    "clean" ) CLEAN=true;;
    "-j"* ) JOBS=$arg;;
    *"qmake" ) QMAKE=$arg
      QT_LIB_DIR=`readlink -e $(dirname $arg)/../lib`
      ;;
  esac
done

WORK_DIR=../../build/$PLATFORM
MINGW_DIR=/opt/mingw-4.9.3/mingw-w64-i686
if [ "$PLATFORM" == "mingw" ]; then
  QT_LIB_DIR=`readlink -e $QT_LIB_DIR/../bin`
  HOST=i686-w64-mingw32
  HOST_ATTR="--host=$HOST"
  STRIP=$HOST-strip
  export PATH="$MINGW_DIR/bin:$PATH"
  COMPILERS="CC=\"${HOST}-gcc\" AR=\"${HOST}-ar\" RANLIB=\"${HOST}-ranlib\""
fi

WORK_DIR=`readlink -m $WORK_DIR`
DEPS_DIR=`readlink -m $WORK_DIR/deps`
APP_DIR=`readlink -m $WORK_DIR/app`
DEB_DIR=`readlink -m $WORK_DIR/deb`
ISS_DIR=`readlink -m $WORK_DIR/iss`
SF_DIR=`readlink -m $WORK_DIR/sf`
DOWNLOAD_DIR=`readlink -m ../../`
SRC_DIR=`readlink -e ..`
SRC_DISTR_DIR=$SRC_DIR/distr

mkdir -p $WORK_DIR

function cleanupDirInNeeded {
  local DIR=$1
  if [[ "$CLEAN" == "true" && -d "$DIR" ]]; then    
    rm -rf $DIR    
  fi
  mkdir -p $DIR
}

