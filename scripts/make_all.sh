#!/bin/bash

source ./options.sh $@
cleanupDirInNeeded $WORK_DIR

echo "Running all scripts for $PLATFORM"
#./install_deps $@
#./get_tessdata.sh $@
#./get_deps.sh $@
./make_deps.sh $@
./make_app.sh $@
if [ "$PLATFORM" == "linux" ]; then
  ./make_deb.sh $@
fi
if [ "$PLATFORM" == "mingw" ]; then
  ./make_iss.sh $@
fi
./make_sf.sh $@
echo "All scripts successfully run"
