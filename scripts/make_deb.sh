#!/bin/bash

source ./options.sh $@

cleanupDirInNeeded $DEB_DIR

cp -r $SRC_DISTR_DIR/deb/* $DEB_DIR
INSTALL_DIR=/opt/ScreenTranslator

function makeChangelog {
local FOUND=false
local CHANGELOG="screen-translator ($VERSION) stable; urgency=medium\n\n"
while read line; do
  if [[ $line =~ [0-9]+\.[0-9]+\.[0-9]+: ]]; then #version info
    if [ "$line" == "$VERSION:" ]; then
      FOUND=true
      continue
    elif [ $FOUND == true ]; then
      break;
    fi
  fi
  if [ $FOUND == true ]; then
    CHANGELOG="$CHANGELOG$line\n\n"
  fi
done < $SRC_DISTR_DIR/Changelog_en.txt
CHANGELOG="$CHANGELOG--Gres <translator@gres.biz> `date -R`\n\n"
echo $CHANGELOG
}

echo "Making deb"
cp $SRC_DIR/LICENSE.md $DEB_DIR/DEBIAN/copyright
mkdir -p $DEB_DIR/usr/share/pixmaps/
cp $SRC_DIR/images/STIconBlue.png $DEB_DIR/usr/share/pixmaps/ScreenTranslator.png
mkdir -p $DEB_DIR/$INSTALL_DIR/
cp $APP_DIR/ScreenTranslator $DEB_DIR/$INSTALL_DIR/ScreenTranslator
$STRIP -s $DEB_DIR/$INSTALL_DIR/ScreenTranslator
cp -r $SRC_DIR/translators $DEB_DIR/$INSTALL_DIR/translators

VERSION=`grep "versionString" $SRC_DIR/version.json | cut -d'"' -f4`
sed "s/Version:.*\+/Version: $VERSION/" -i $DEB_DIR/DEBIAN/control
sed "s/Version=.*\+/Version=$VERSION/" -i $DEB_DIR/usr/share/applications/ScreenTranslator.desktop
SIZE=$(expr `du -bs $DEB_DIR | cut -f1` / 1024)
sed "s/Installed-Size:.*\+/Installed-Size: $SIZE/" -i $DEB_DIR/DEBIAN/control
echo -e $(makeChangelog) > $DEB_DIR/DEBIAN/changelog

fakeroot dpkg-deb --build $DEB_DIR $WORK_DIR/screen-translator-$VERSION.deb

