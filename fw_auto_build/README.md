## Firmware building

The Dockerfile in this directory is used to build the image ```prelz/mvm_fw_build```, which can be found in Docker Hub.

The image is a Debian-based container, with an installation of [```arduino-cli```](https://arduino.github.io/arduino-cli/) and all the dependencies required to compile the firmware for *MVM*. The script to compile the firmware is also provided, at path ```/usr/local/bin/mvm_fw_build.sh```.

If you have *Docker* installed and running, you can build the firmware in the current directory with the command:

    $ docker run -v `pwd`:/mnt prelz/mvm_fw_build mvm_fw_build.sh [FW version tag]

If you want to play around with compilation options, you can either edit the script and rebuild the container image, or open a shell in the existing container and modify the script there (note that changes will be lost if you delete the container).
