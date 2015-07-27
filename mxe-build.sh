#!/bin/sh

if [ $# -lt 2 ] ; then
  echo "Usage: $0 <path-to-mxe-environment> <path-to-directory-containing-libcomm14cux-dll>"
  exit
fi

MXE=$1
LIBCOMM14CUX=$2
MXETYPE="i686-w64-mingw32.shared"

cp $2/libcomm14cux.dll              $1/usr/$MXETYPE/lib/libcomm14cux.dll
cp /usr/include/comm14cux.h         $1/usr/$MXETYPE/include/comm14cux.h
cp /usr/include/comm14cux_version.h $1/usr/$MXETYPE/include/comm14cux_version.h

export PATH=$PATH:$2:$1/usr/$MXETYPE/qt5/bin:$1/usr/$MXETYPE/qt5/plugins/platforms:$MXE/usr/$MXETYPE/lib:$MXE/usr/$MXETYPE/qt5/lib:$MXE/usr/lib/gcc/$MXETYPE/5.1.0/

cmake .. -DCMAKE_TOOLCHAIN_FILE=$MXE/usr/$MXETYPE/share/cmake/mxe-conf.cmake -DCMAKE_PREFIX_PATH=$MXE/usr/$MXETYPE/qt5/

