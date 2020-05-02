#
# File:     Dockerfile
# Author:   Francesco Prelz (francesco.prelz@mi.infn.it)
#
# Revision history:
#     30-Apr-2020: Original release
#
# Description:
#     Gather all dependencies (Arduino amd friends) to build the MVM
#     firmware.
#

FROM debian:stretch

ADD "mvm_fw_build.sh" "/usr/local/bin"

RUN apt-get update && apt-get install -y wget git xz-utils sudo \
                                         python python-serial

WORKDIR "/root"

RUN wget --quiet -O arduino.tar.xz\
      "https://www.arduino.cc/download.php?f=/arduino-1.8.12-linux64.tar.xz";\
    xzcat arduino.tar.xz | tar -C /usr/local -xf -
RUN /usr/local/arduino-1.8.12/install.sh; rm -f arduino.tar.xz;\
    wget --quiet -O arduino-cli.tar.gz\
      "https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Linux_64bit.tar.gz";\
    tar -C /usr/local/bin -zxvf arduino-cli.tar.gz arduino-cli;\
    chmod +x /usr/local/bin/arduino-cli;\
    chmod +x /usr/local/bin/mvm_fw_build.sh;\
    rm -f arduino-cli.tar.gz
RUN /usr/local/bin/arduino-cli core --additional-urls\
      "https://dl.espressif.com/dl/package_esp32_index.json" update-index;\
    /usr/local/bin/arduino-cli core --additional-urls\
      "https://dl.espressif.com/dl/package_esp32_index.json"\
       install esp32:esp32@1.0.4;\
    /usr/local/bin/arduino-cli lib install SimpleCli;\
    ln -f /root/.arduino15/packages/esp32/hardware/esp32/1.0.4/cores/esp32/Arduino.h /root/.arduino15/packages/esp32/hardware/esp32/1.0.4/cores/esp32/arduino.h

ENTRYPOINT []
