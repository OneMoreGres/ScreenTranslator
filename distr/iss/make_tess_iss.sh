#!/bin/bash

TESSDATA_DIR="$1"

if [ -z "$TESSDATA_DIR" ]; then echo "Usage $0 <tessdata_dir>"; exit 0; fi

TESSDATA_DIR=`readlink -e $TESSDATA_DIR`
OUT_FILE="tessdata.iss"
OUT_FILE=`readlink -m $OUT_FILE`
LANGS_FILE="code2langTr.txt"

function getLangsOrder {
  local FIELD=$1
  local ORDER=`cat $LANGS_FILE | cut -d' ' -f$FIELD | sort`
  local FNAMES=""
  for i in $ORDER; do
    local FNAME=`grep "$i" $LANGS_FILE | cut -d' ' -f1`
    if [[ -z "$FNAME" || "${FNAME:0:1}" == "#" ]]; then
      continue;
    fi
    FNAMES=$FNAMES" $FNAME"
  done
  echo $FNAMES
}

FILES="[Files]\n"
COMPONENTS="[Components]\nName: \"Languages\"; Description: \"{cm:Languages}\"; Types: custom\n"
MESSAGES_EN="\n[CustomMessages]\nen.Languages=OCR Languages\n"
MESSAGES_RU="\n[CustomMessages]\nru.Languages=Языки распознавания\n"
PREV_LANG=""
CUSTOM_LANGS="eng"


function fillIss {
  local LANG_FIELD=$1
  local COMPONENT_LANG=$2
  local ONLY_COMPONENTS=$3
  COMPONENTS=$COMPONENTS"\n"
  local ORDER=`cat $LANGS_FILE | cut -d' ' -f$LANG_FIELD | sort`
  for i in $ORDER; do
    local LANG_LINE=$(grep " $i " $LANGS_FILE)
    local FNAME=$(echo "$LANG_LINE" | cut -d' ' -f1)
    local LANG_EN=$(echo "$LANG_LINE" | cut -d' ' -f2)
    local LANG_RU=$(echo "$LANG_LINE" | cut -d' ' -f3)
    if [[ -z "$FNAME" || "${FNAME:0:1}" == "#" ]]; then
      continue;
    fi

    local CUR_LANG_FILES=`find $TESSDATA_DIR -name "$FNAME.*"`
    if [ -z "$CUR_LANG_FILES" ]; then
      echo "no lang"
      continue
    fi


    local COMPONENT_SIZE=0
    for s in `find $TESSDATA_DIR -name "$FNAME.*" -exec wc -c {} \; | cut -d' ' -f1`; do
      COMPONENT_SIZE=$(expr $COMPONENT_SIZE + $s)
    done
    TYPES=""
    if [[ $CUSTOM_LANGS =~ $FNAME ]]; then
      TYPES="custom"
    fi
    if [ ! -z "$TYPES" ]; then
      TYPES="Types: $TYPES; ";
    fi
    COMPONENTS=$COMPONENTS"Name: \"Languages\\\\$LANG_EN\"; Description: \"{cm:$LANG_EN}\"; Languages: $COMPONENT_LANG;
      $TYPES ExtraDiskSpaceRequired: $COMPONENT_SIZE\n"


    if $ONLY_COMPONENTS; then
      continue;
    fi
    MESSAGES_EN=$MESSAGES_EN"en.$LANG_EN=$(echo "$LANG_EN" | sed 's/_/ /g')\n"
    MESSAGES_RU=$MESSAGES_RU"ru.$LANG_EN=$(echo "$LANG_RU" | sed 's/_/ /g')\n"

    for f in $CUR_LANG_FILES; do
      local FNAME=$(basename "$f")
      FILES=$FILES"Source: \"{tmp}\\\\$FNAME\"; DestDir: \"{app}\\\\tessdata\"; Components: Languages\\\\$LANG_EN;
	Flags: external; Check: DwinsHs_Check(ExpandConstant('{tmp}\\\\$FNAME'),
	  'https://github.com/tesseract-ocr/tessdata/raw/3.04.00/$FNAME', 'ST_setup', 'Get', 0);\n"
    done
  done
}
fillIss 2 "en" false
fillIss 3 "ru" true

echo -e $FILES > $OUT_FILE
echo -e $COMPONENTS >> $OUT_FILE
echo -e $MESSAGES_EN >> $OUT_FILE
echo -e $MESSAGES_RU >> $OUT_FILE
iconv -f utf8 -t cp1251 $OUT_FILE -o $OUT_FILE.1
mv $OUT_FILE.1 $OUT_FILE

