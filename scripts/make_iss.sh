#!/bin/bash

source ./options.sh $@

cleanupDirInNeeded $ISS_DIR

cp -r $SRC_DISTR_DIR/iss/* $ISS_DIR
CONTENT_DIR=$ISS_DIR/content
mkdir -p $CONTENT_DIR

echo "Making ISS"
TESSDATA_DIR="$DOWNLOAD_DIR/tessdata"
$(cd $ISS_DIR && ./make_tess_iss.sh $TESSDATA_DIR out="$ISS_DIR/tessdata.iss")

#setup
VERSION=`grep "versionString" $SRC_DIR/version.json | cut -d'"' -f4`
sed "s/#define MyAppVersion.*$/#define MyAppVersion \"$VERSION\"/" -i $ISS_DIR/InnoSetup.iss
cp $SRC_DIR/images/icon.ico $ISS_DIR/icon.ico
cp $SRC_DIR/LICENSE.md $ISS_DIR/LICENSE_en.md
cp $SRC_DISTR_DIR/Changelog_en.txt $ISS_DIR/Changelog_en.txt
cp $SRC_DISTR_DIR/Changelog_ru.txt $ISS_DIR/Changelog_ru.txt

#app
cp $APP_DIR/ScreenTranslator.exe $CONTENT_DIR/ScreenTranslator.exe
cp -r $SRC_DIR/translators $CONTENT_DIR/translators

#libs
QT_LIBS="Qt5WebKitWidgets Qt5Widgets Qt5WebKit Qt5Gui Qt5Network Qt5Core Qt5Sensors Qt5Positioning Qt5PrintSupport 
Qt5OpenGL Qt5Sql Qt5Quick Qt5Qml Qt5WebChannel Qt5Multimedia Qt5MultimediaWidgets"
for i in $QT_LIBS; do
  cp -d $QT_LIB_DIR/$i.dll $CONTENT_DIR
done

mkdir -p $CONTENT_DIR/platforms
cp -d $QT_LIB_DIR/../plugins/platforms/qwindows.dll $CONTENT_DIR/platforms

MINGW_LIBS="libgcc_s_sjlj-1 libstdc++-6 icuin55 icuuc55 icudt55 libwinpthread-1 ssleay32 libeay32"
for i in $MINGW_LIBS; do
  cp -d $MINGW_DIR/lib/$i.dll $CONTENT_DIR
done

cp -d $DEPS_DIR/lib/liblept*.dll $CONTENT_DIR
cp -d $DEPS_DIR/lib/libtesseract*.dll $CONTENT_DIR

find $CONTENT_DIR -name '*.exe' -exec $STRIP -s {} \;
find $CONTENT_DIR -name '*.dll' -exec $STRIP -s {} \;

cd $ISS_DIR
wine "C:\Program Files\Inno Setup 5\iscc.exe" "InnoSetup.iss"
