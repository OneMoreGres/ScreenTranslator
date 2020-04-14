#!/bin/sh

pack() {
  mkdir -p "$2"
  for f in $(ls $1); do
    source="$1/$f"
    target="$2/$f"
    if [ -d "$source" ]; then
      pack "$source" "$target"
    elif [ -f "$source" ]; then
      if [ "$target.zip" -nt "$source" ]; then
        echo "$source is up to date"
        continue
      fi
      tmp=/tmp/archive.zip
      echo "packing $source -> $tmp"
      ls -l "$source"
      zip -9 -j "$tmp" "$source"

      echo "moving $tmp -> $target.zip"
      mv "$tmp" "$target.zip"
      chmod 444 "$target.zip"
      ls -l "$target.zip"
    fi
  done
}

mirror() {
  cur="$(pwd)"
  url="$1"
  dir="$2"
  git_dir="/git/$dir"
  pack_dir="/packed/$dir"
  echo $url $git_dir $pack_dir
  if [ -d $git_dir ]; then
    echo "fetching"
    cd $git_dir && git fetch --depth=1 origin master
  else
    echo "cloning"
    git clone --depth=1 --single-branch "$url" $git_dir
  fi
  echo "packing"
  pack "$git_dir" "$pack_dir"
}

while true; do
  mirror 'git://anongit.freedesktop.org/libreoffice/dictionaries' 'dictionaries'
  mirror 'https://github.com/tesseract-ocr/tessdata_best.git' 'tessdata_best'
  echo "sleeping"
  sleep 6h
done
