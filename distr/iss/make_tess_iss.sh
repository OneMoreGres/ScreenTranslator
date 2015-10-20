#!/bin/bash

TESSDATA_DIR="../../../tessdata"
TESSDATA_TAG="3.04.00"
for arg in ${@}; do
  case "$arg" in
    "tessdata" ) TESSDATA_DIR="$arg";;
    "tag="* ) TESSDATA_TAG=${arg:4};;
  esac
done
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
COMPONENTS="[Components]\nName: \"Languages\"; Description: \"{cm:Languages}\"; Types: custom full\n"
MESSAGES_EN="\n[CustomMessages]\nen.Languages=OCR Languages\n"
MESSAGES_RU="\n[CustomMessages]\nru.Languages=Языки распознавания\n"
PREV_LANG=""
COMPACT_LANGS="eng rus deu spa chi_sim fra jpn"


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
    TYPES="full"
    if [[ $COMPACT_LANGS =~ $FNAME ]]; then
      TYPES="compact custom $TYPES"
    fi
    COMPONENTS=$COMPONENTS"Name: \"Languages\\\\$LANG_EN\"; Description: \"{cm:$LANG_EN}\"; Languages: $COMPONENT_LANG; 
      Types: $TYPES; ExtraDiskSpaceRequired: $COMPONENT_SIZE\n"      
      
      
    if $ONLY_COMPONENTS; then
      continue;
    fi    
    MESSAGES_EN=$MESSAGES_EN"en.$LANG_EN=$(echo "$LANG_EN" | sed 's/_/ /g')\n"
    MESSAGES_RU=$MESSAGES_RU"ru.$LANG_EN=$(echo "$LANG_RU" | sed 's/_/ /g')\n"
    
    for f in $CUR_LANG_FILES; do
      local FNAME=$(basename "$f")
      FILES=$FILES"Source: \"{tmp}\\\\$FNAME\"; DestDir: \"{app}\\\\tessdata\"; Components: Languages\\\\$LANG_EN; 
	Flags: external; Check: DwinsHs_Check(ExpandConstant('{tmp}\\\\$FNAME'), 
	  'https://cdn.rawgit.com/tesseract-ocr/tessdata/$TESSDATA_TAG/$FNAME', 'ST_setup', 'Get', 0);\n"
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

exit 0
function fillIss {
  local ORDER=$1
  local FIELD=$2
  local ONLY_COMPONENTS=$3
  local FILES=""
  for i in $ORDER; do
    local CUR_LANG_FILES=`find $TESSDATA_DIR -name \"$i.*\"`
    if [ -z "$CUR_LANG_FILES" ]; then
      continue
    fi
    FILES=$FILES" `find $TESSDATA_DIR -name \"$i.*\"`"
  done
  for i in $FILES; do
  local fName=$(basename "$i")
  local LANG=$(echo "$fName" | cut -d'.' -f1)
  LANG_LINE=$(grep "$lang " $LANGS_FILE)
  if [[ -z "$LANG_LINE" || "${LANG_LINE:0:1}" == "#" ]]; then
    continue;
  fi
  LANG_EN=$(echo "$LANG_LINE" | cut -d' ' -f2)
  LANG_RU=$(echo "$LANG_LINE" | cut -d' ' -f3)
  if ! $ONLY_COMP; then
  FILES=$FILES"Source: \"{tmp}\\\\$fName\"; DestDir: \"{app}\\\\tessdata\"; Components: Languages\\\\$LANG_EN; 
  Flags: external; Check: DwinsHs_Check(ExpandConstant('{tmp}\\\\$fName'), 
    'https://cdn.rawgit.com/tesseract-ocr/tessdata/$TESSDATA_TAG/$fName', 'ST_setup', 'Get', 0);\n"
  fi
  if [ "$PREV_LANG" != "$LANG_EN" ]; then
    PREV_LANG="$LANG_EN"
    SIZE=0
    for s in `find $TESSDATA_DIR -name "$lang.*" -exec wc -c {} \; | cut -d' ' -f1`; do
      SIZE=$(expr $SIZE + $s)
    done
    TYPES="full"
    if [[ $COMPACT_LANGS =~ $lang ]]; then
      TYPES="compact custom $TYPES"
    fi
    COMPONENTS=$COMPONENTS"Name: \"Languages\\\\$LANG_EN\"; Description: \"{cm:$LANG_EN}\"; Languages: $LLANG; Types: $TYPES; ExtraDiskSpaceRequired: $SIZE\n"
    
  if ! $ONLY_COMP; then
    MESSAGES_EN=$MESSAGES_EN"en.$LANG_EN=$(echo "$LANG_EN" | sed 's/_/ /g')\n"
    MESSAGES_RU=$MESSAGES_RU"ru.$LANG_EN=$(echo "$LANG_RU" | sed 's/_/ /g')\n"
  fi
  fi
  done
}
echo $(fillIss "$(getLangsOrder 2)" "en" false)



function doJob {
LLANG=$2
ONLY_COMP=$3
for fff in $1; do
  for f in `find $TESSDATA_DIR -name "$fff.*"`; do
  f=$(basename "$f")
  lang=$(echo "$f" | cut -d'.' -f1)
  LANG_LINE=$(grep "$lang " $LANGS_FILE)
  if [[ -z "$LANG_LINE" || "${LANG_LINE:0:1}" == "#" ]]; then
    continue;
  fi
  LANG_EN=$(echo "$LANG_LINE" | cut -d' ' -f2)
  LANG_RU=$(echo "$LANG_LINE" | cut -d' ' -f3)
  if ! $ONLY_COMP; then
  FILES=$FILES"Source: \"{tmp}\\\\$f\"; DestDir: \"{app}\\\\tessdata\"; Components: Languages\\\\$LANG_EN; 
  Flags: external; Check: DwinsHs_Check(ExpandConstant('{tmp}\\\\$f'), 
    'https://cdn.rawgit.com/tesseract-ocr/tessdata/$TESSDATA_TAG/$f', 'ST_setup', 'Get', 0);\n"
  fi
  if [ "$PREV_LANG" != "$LANG_EN" ]; then
    PREV_LANG="$LANG_EN"
    SIZE=0
    for s in `find $TESSDATA_DIR -name "$lang.*" -exec wc -c {} \; | cut -d' ' -f1`; do
      SIZE=$(expr $SIZE + $s)
    done
    TYPES="full"
    if [[ $COMPACT_LANGS =~ $lang ]]; then
      TYPES="compact custom $TYPES"
    fi
    COMPONENTS=$COMPONENTS"Name: \"Languages\\\\$LANG_EN\"; Description: \"{cm:$LANG_EN}\"; Languages: $LLANG; Types: $TYPES; ExtraDiskSpaceRequired: $SIZE\n"
    
  if ! $ONLY_COMP; then
    MESSAGES_EN=$MESSAGES_EN"en.$LANG_EN=$(echo "$LANG_EN" | sed 's/_/ /g')\n"
    MESSAGES_RU=$MESSAGES_RU"ru.$LANG_EN=$(echo "$LANG_RU" | sed 's/_/ /g')\n"
  fi
  fi
  done
done
}
doJob "$(getOrder 2)" "en" false
doJob "$(getOrder 3)" "ru" true
echo $COMPONENTS
echo -e $FILES > $OUT_FILE
echo -e $COMPONENTS >> $OUT_FILE
echo -e $MESSAGES_EN >> $OUT_FILE
echo -e $MESSAGES_RU >> $OUT_FILE
iconv -f utf8 -t cp1251 $OUT_FILE -o $OUT_FILE.1
mv $OUT_FILE.1 $OUT_FILE


exit 0


for f in `ls $TESSDATA_DIR | sort`; do
  lang=$(echo "$f" | cut -d'.' -f1)
  LANG_LINE=$(grep "$lang " $LANGS_FILE)
  if [[ -z "$LANG_LINE" || "${LANG_LINE:0:1}" == "#" ]]; then
    continue;
  fi
  LANG_EN=$(echo "$LANG_LINE" | cut -d' ' -f2)
  LANG_RU=$(echo "$LANG_LINE" | cut -d' ' -f3)
  FILES=$FILES"Source: \"{tmp}\\\\$f\"; DestDir: \"{app}\\\\tessdata\"; Components: Languages\\\\$LANG_EN; 
  Flags: external; Check: DwinsHs_Check(ExpandConstant('{tmp}\\\\$f'), 
    'https://cdn.rawgit.com/tesseract-ocr/tessdata/$TESSDATA_TAG/$f', 'ST_setup', 'Get', 0);\n"
  if [ "$PREV_LANG" != "$LANG_EN" ]; then
    PREV_LANG="$LANG_EN"
    SIZE=0
    for s in `find $TESSDATA_DIR -name "$lang.*" -exec wc -c {} \; | cut -d' ' -f1`; do
      SIZE=$(expr $SIZE + $s)
    done
    TYPES="full"
    if [[ $COMPACT_LANGS =~ $lang ]]; then
      TYPES="compact custom $TYPES"
    fi
    COMPONENTS=$COMPONENTS"Name: \"Languages\\\\$LANG_EN\"; Description: \"{cm:$LANG_EN}\"; Types: $TYPES; ExtraDiskSpaceRequired: $SIZE\n"
    MESSAGES_EN=$MESSAGES_EN"en.$LANG_EN=$(echo "$LANG_EN" | sed 's/_/ /g')\n"
    MESSAGES_RU=$MESSAGES_RU"ru.$LANG_EN=$(echo "$LANG_RU" | sed 's/_/ /g')\n"
  fi
done

echo -e $FILES > $OUT_FILE
echo -e $COMPONENTS >> $OUT_FILE
echo -e $MESSAGES_EN >> $OUT_FILE
echo -e $MESSAGES_RU >> $OUT_FILE
iconv -f utf8 -t cp1251 $OUT_FILE -o $OUT_FILE.1
mv $OUT_FILE.1 $OUT_FILE
