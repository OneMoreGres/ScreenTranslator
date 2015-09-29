#!/bin/bash

WORK_DIR=linux

#WORK_DIR=mingw
#HOST=i686-w64-mingw32
#export PATH="/opt/mingw-4.9.3/mingw-w64-i686/bin:$PATH"

#rm -rf $WORK_DIR
mkdir $WORK_DIR 2> /dev/null
set -e

WORK_DIR=`realpath $WORK_DIR`
if [ ! -z "$HOST" ]; then HOST_ATTR="--host=$HOST"; fi

echo "Building Leptonica"
tar zxf leptonica.tar.gz -C $WORK_DIR
LEPT_DIR=`ls $WORK_DIR | grep lept`
cd $WORK_DIR/$LEPT_DIR
./configure --prefix=$WORK_DIR --disable-programs $HOST_ATTR
make -j4
make install
cd ../..

echo "Building Tesseract"
tar zxf tesseract.tar.gz -C $WORK_DIR
TESS_DIR=`ls $WORK_DIR | grep tess`
cd $WORK_DIR/$TESS_DIR
sed 's/vs2008/vs2010/g' -i ccutil/Makefile.in
sed 's/Windows\.h/windows\.h/g' -i opencl/openclwrapper.cpp
LIBLEPT_HEADERSDIR=$WORK_DIR/include LIBS=-L$WORK_DIR/lib ./configure --prefix=$WORK_DIR --disable-tessdata-prefix $HOST_ATTR
make -j4
make install
cd ../..

find -name '*.dll' -exec cp {} $WORK_DIR/lib \;

echo "Done"
