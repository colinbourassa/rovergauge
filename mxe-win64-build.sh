#!/bin/sh

if [ $# -lt 1 ] ; then
  echo "Usage: $0 <path-to-mxe-environment>"
  exit
fi

MXETYPE="x86_64-w64-mingw32.shared"
MXE="$1/usr/$MXETYPE"

if [ ! -d "$MXE" ] ; then
  echo "Error: This script currently only supports x86_64 shared library builds ($MXETYPE)."
  echo "Please update your MXE settings.mk file to include this target type and then build Qt5 and libcomm14cux."
  exit 1
fi

if [ ! -d "$MXE/qt5" ] ; then
  echo "Error: Qt5 must first be built within MXE (expected at: $MXE/qt5)"
  exit 2
fi

if [ ! -f "$MXE/bin/libcomm14cux.dll" ] || [ ! -f "$MXE/include/comm14cux.h" ] ; then
  echo "Error: libcomm14cux must first be built within MXE."
  exit 3
fi

export PATH=$PATH:$MXE/bin:$MXE/qt5/bin:$MXE/qt5/plugins/platforms:$MXE/lib:$MXE/qt5/lib:$1/usr/lib/gcc/$MXETYPE/5.1.0/

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$MXE/share/cmake/mxe-conf.cmake -DCMAKE_PREFIX_PATH=$MXE/qt5/

