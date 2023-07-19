#!/bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SRCDIR=$DIR/code
DESTDIR=$DIR/prebuilt/mac
export MACOSX_DEPLOYMENT_TARGET="10.13"

rm "$DESTDIR"/*.a

cd $SRCDIR
make clean
make CC="gcc -m64 -arch x86_64" clean all

if [ -f $SRCDIR/src/libluajit.a ]; then
    mv $SRCDIR/src/libluajit.a $DESTDIR/libluajit.a
fi;

make clean
