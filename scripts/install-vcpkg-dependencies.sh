#!/bin/sh

# Adapted from https://wiki.qt.io/Building_Qt_6_from_Git:

sudo apt install build-essential git cmake ninja-build perl python3

# Adapted from https://doc.qt.io/qt-6/linux-requirements.html:

sudo apt install libfontconfig1-dev libfreetype-dev libx11-dev libx11-xcb-dev libxext-dev libxfixes-dev \
    libxi-dev libxrender-dev libxcb1-dev libxcb-cursor-dev libxcb-glx0-dev libxcb-keysyms1-dev libxcb-image0-dev \
    libxcb-shm0-dev libxcb-icccm4-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-randr0-dev \
    libxcb-render-util0-dev libxcb-util-dev libxcb-xinerama0-dev libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev

# Just in case the above command is not enough, this message is printed by vcpkg:
# qtbase currently requires packages from the system package manager.  They
# can be installed on Ubuntu systems via sudo apt-get install '^libxcb.*-dev' 
# libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev
# libxkbcommon-x11-dev libegl1-mesa-dev

# Prevent:
# - "[icu] build failure" (https://github.com/microsoft/vcpkg/issues/38005)
# - "[icu] Add reminder to install autoconf-archive" (https://github.com/microsoft/vcpkg/pull/38019)

sudo apt install autoconf-archive

# Not sure whether this package is needed for Qt6 but the build failed without it:

sudo apt install python3-jinja2 python3-venv
