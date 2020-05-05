#!/bin/sh
#
# File:     mwm_fw_build.sh
# Author:   Francesco Prelz (francesco.prelz@mi.infn.it)
#
# Revision history:
#     30-Apr-2020: Original release
#
# Description:
#     Script to pull from GIT and build the MVM firmware, once all
#     the build dependencies are in place (e.g. in the debian-mvm-fw-build)
#     docker image:
#     docker run -v /host/build/directory/:/mnt prelz/mvm-fw-build mwm_fw_build.sh [tag - e.g. "V5"]
#

fw_tag=${1:-""};
git_proj="https://github.com/NuclearInstruments/MVMFirmwareCpp.git"
exitcode=1

cd /mnt

echo "$0: git cloning repository at $git_proj ... " >&2
git clone -q "$git_proj"

if [ $? -eq 0 ]; then
  echo "Done" >&2
  cd MVMFirmwareCpp
  if [ "x$fw_tag" != "x" ]; then
    echo "$0: git checking out \"$fw_tag\" tag ... " >&2
    if git checkout -q $fw_tag; then
      echo "Done" >&2
    else
      echo "ERROR - will use master branch" >&2
    fi
  fi
  echo "$0: compiling arduino-cli ... " >&2
  arduino-cli compile \
    --fqbn  "esp32:esp32:featheresp32:FlashFreq=80,UploadSpeed=115200,DebugLevel=none,PartitionScheme=default" \
    MVMFirmwareCore
  exitcode=$?
  if [ $exitcode -eq 0 ]; then
    echo "Done" >&2
  else
    echo "ERROR" >&2 
  fi
else
  echo "ERROR" >&2 
fi

exit $exitcode
