#!/bin/sh

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SRCDIR=$DIR/code
DESTDIR=$DIR/prebuilt/linux

rm "$DESTDIR"/*.a

cd $SRCDIR
make clean
make CC="gcc" clean all

if [ -f $SRCDIR/src/libluajit.a ]; then
    mv $SRCDIR/src/libluajit.a $DESTDIR/libluajit.a
fi;

make clean