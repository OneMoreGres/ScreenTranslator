#!/bin/bash

source ./options.sh $@

cleanupDirInNeeded $SF_DIR

echo "Making SF"
function format {
  local FILE=$1
  sed "s/\(Changes\)/#\1/g" -i $FILE
  sed "s/\(Изменения\)/#\1/g" -i $FILE
  sed "s/\([0-9]\+\.[0-9]\+\.[0-9]\+:\)/##\1/g" -i $FILE
  sed "s/+\s/* /g" -i $FILE
  sed "s/-\s/* /g" -i $FILE
}
cp -f $SRC_DISTR_DIR/Changelog_en.txt $SF_DIR/readme.md
format $SF_DIR/readme.md
cp -f $SRC_DISTR_DIR/Changelog_ru.txt $SF_DIR/readme_ru.md
iconv -f cp1251 -t utf8 $SF_DIR/readme_ru.md -o $SF_DIR/readme_ru.md.u
mv $SF_DIR/readme_ru.md.u $SF_DIR/readme_ru.md
format $SF_DIR/readme_ru.md
