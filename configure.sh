#!/usr/bin/env sh

# Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
# Licensed under GPLv3, see LICENSE for more information.

srcdir="src"
VERSION=$(cat "${srcdir}/VERSION")
CONFIG_HEADER="${srcdir}/config.h"


# To enable debug, set this to zero
DEBUG=1

BINFILE="hcal"
CC="gcc"
CFLAGS="-g -Wall -Werror -pedantic -std=c99"
LIBS="-lncursesw -lvector"

topdir="$PWD"

if [ $DEBUG -eq 0 ] ; then
    CFLAGS="$CFLAGS -g"
fi

HAS_NCURSESW=1

function addtoconfig {
    echo "$1" >> $CONFIG_HEADER
}

alias atc='addtoconfig'

function hasncursesw {
    $(pkg-config --exists ncursesw)
    if [ $? -eq 0 ] ; then
        HAS_NCURSESW=0
    else
        return 1
    fi
}

function write_config_h {
# If $CONFIG_HEADER exists, empty it
    if [ -f $CONFIG_HEADER ] ;then
        rm $CONFIG_HEADER
        touch $CONFIG_HEADER
    fi

    atc "#ifndef CONFIG_H"
    atc "#define CONFIG_H"
    atc ""

    atc "#define _POSIX_C_SOURCE 200809L"
    atc ""

    atc "#define NAME \"$BINFILE\""
    atc "#define VERSION \"$VERSION\""

    if [ $DEBUG -eq 0 ] ; then
        atc "#define DEBUG 1"
    else
        atc "#define DEBUG 0"
    fi

    atc ""
    atc ""
    atc "#endif /* CONFIG_H */"
    atc ""
}


if [ ! hasncursesw ] ; then
    echo "Can't find ncursesw, aborting...">&2
    exit 1
else
    echo "Found ncursesw"
fi

if [ $DEBUG -eq 0 ] ; then
    echo "Debug is enabled"
else
    echo "Debug is disabled"
fi

write_config_h


source ${srcdir}/Makefile.def

phony_target_list="all"

mf="Makefile"
makefile="${srcdir}/${mf}"

cd $srcdir
rm -f $mf
touch $mf

echo "CC=gcc" >> $mf
echo "CFLAGS=$CFLAGS" >> $mf
echo "" >> $mf
echo "all: $BINFILE" >> $mf

objlist=""
for sfile in $SOURCE_FILES ; do
    objlist="$objlist ${sfile/%.c/.o}"
done

echo "" >> $mf
echo "$BINFILE: $objlist" >> $mf
echo -e "\t\$(CC) $objlist $LIBS -o $BINFILE" >> $mf
echo >> $mf

for sfile in $SOURCE_FILES ; do
    ${CC} -MM $sfile >> $mf
    echo -e "\t\$(CC) -c \$(CFLAGS) $sfile -o ${sfile/%.c/.o}" >> $mf
    echo "" >> $mf
done

echo "" >> $mf

for hfile in $GENERATED_HEADERS ; do
    echo "$hfile:" >> $mf
    echo "" >> $mf
done

echo "" >> $mf
echo "" >> $mf

echo "clean:" >> $mf
echo -e "\t-rm -f $objlist" >> $mf
echo -e "\t-rm -f $BINFILE" >> $mf
echo -e "\t-rm -f $GENERATED_HEADERS" >> $mf
echo "" >> $mf

echo ".PHONY: all clean" >> $mf
echo "" >> $mf

cd $topdir

