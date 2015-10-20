#!/bin/bash

source ./options.sh $@

BUILD_LEPT=false
BUILD_TESS=false

for arg in ${@}; do
  case "$arg" in
    "tess" ) BUILD_TESS=true;;
    "lept" ) BUILD_LEPT=true;;
  esac
done

if ! $BUILD_LEPT && ! $BUILD_TESS ; then
  BUILD_LEPT=true
  BUILD_TESS=true
fi

cleanupDirInNeeded $DEPS_DIR


if $BUILD_LEPT ; then
  echo "Building Leptonica"
  tar zxf $DOWNLOAD_DIR/leptonica.tar.gz -C $DEPS_DIR
  LEPT_DIR=`ls $DEPS_DIR | grep lept`
  pushd .
  cd $DEPS_DIR/$LEPT_DIR
  $CFLAGS ./configure --prefix=$DEPS_DIR --disable-programs "$HOST_ATTR"
  make $JOBS
  make install
  popd
fi

if $BUILD_TESS ; then
  echo "Building Tesseract"
  tar zxf $DOWNLOAD_DIR/tesseract.tar.gz -C $DEPS_DIR
  TESS_DIR=`ls $DEPS_DIR | grep tess`
  pushd .
  cd $DEPS_DIR/$TESS_DIR
  sed 's/vs2008/vs2010/g' -i ccutil/Makefile.in
  sed 's/Windows\.h/windows\.h/g' -i opencl/openclwrapper.cpp
  LIBLEPT_HEADERSDIR=$DEPS_DIR/include LIBS=-L$DEPS_DIR/lib ./configure --prefix=$DEPS_DIR --disable-tessdata-prefix "$HOST_ATTR"
  make $JOBS
  make install
  popd
fi

if [ $PLATFORM == 'mingw' ]; then
  find $DEPS_DIR -name '*.dll' -exec cp {} $DEPS_DIR/lib \;
fi

echo "Done"

