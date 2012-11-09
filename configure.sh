#!/usr/bin/env sh

# Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
# Licensed under GPLv3, see LICENSE for more information.

srcdir="src"
VERSION=$(cat "${srcdir}/VERSION")
CONFIG_HEADER="${srcdir}/config.h"


# To enable debug, set this to zero
DEBUG=0

PREFIX="/usr/local"
BINFILE="ccal"
CC="gcc"
CFLAGS="-g -Wall -Wextra -pedantic -std=c99"
LIBS="-lvector"
INCLUDES=""

topdir="$PWD"

if [ $DEBUG -eq 0 ] ; then
    CFLAGS="$CFLAGS -g"
fi

HAS_NCURSESW=1

function add_to_config {
    echo "$1" >> $CONFIG_HEADER
}

alias atc='add_to_config'

function have_ncursesw {
    $(pkg-config --exists ncursesw)
    if [ $? -eq 0 ] ; then
        HAS_NCURSESW=0
    else
        HAS_NCURSESW=1
    fi

    return $HAS_NCURSESW
}


function get_ncurses_flags {
    local inc=$(pkg-config --cflags ncursesw)
    local lib=$(pkg-config --libs ncursesw)

    INCLUDES="$INCLUDES $inc"
    LIBS="$LIBS $lib"
}


function have_program {
    if [ -z "$1" ] ; then
        echo "aoeu"
        return 1
    fi

    which $1 > /dev/null 2>&1
    return $?
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
    atc "#define _XOPEN_SOURCE"
    atc "#define _GNU_SOURCE 1"
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

    return 0
}


function create_makefile {
    local mf="Makefile"

    rm -f $mf
    touch $mf

    echo "PREFIX=$PREFIX" >> $mf
    echo "BIN=$BINFILE" >> $mf
    echo "SRCDIR=src" >> $mf
    echo "" >> $mf

    echo "all: \$(SRCDIR)/\$(BIN)" >> $mf
    echo "" >> $mf

    echo "\$(SRCDIR)/\$(BIN):" >> $mf
	echo -e "\t\$(MAKE) --directory=\$(SRCDIR)" >> $mf
    echo "" >> $mf

    echo "install: \$(SRCDIR)/\$(BIN)" >> $mf
	echo -e "\tinstall -m 0755 \$(SRCDIR)/\$(BIN) \$(PREFIX)/bin/\$(BIN)" >> $mf
    echo "" >> $mf

    echo "uninstall:" >> $mf
	echo -e "\trm \$(PREFIX)/bin/\$(BIN)" >> $mf
    echo "" >> $mf

    echo "clean:" >> $mf
	echo -e "\t\$(MAKE) --directory=\$(SRCDIR) clean" >> $mf
    echo "" >> $mf

    echo "distclean: clean" >> $mf
	echo -e "\t\$(MAKE) --directory=\$(SRCDIR) distclean" >> $mf
	echo -e "\trm -f Makefile" >> $mf
    echo "" >> $mf

    echo ".PHONY: all clean distclean install uninstall" >> $mf

    return 0
}


function create_src_makefile {
    source ${srcdir}/Makefile.def

    phony_target_list="all"

    mf="Makefile"
    makefile="${srcdir}/${mf}"

    cd $srcdir
    rm -f $mf
    touch $mf

    echo "CC=gcc" >> $mf
    echo "CFLAGS=$CFLAGS" >> $mf
    echo "LIBS=$LIBS" >> $mf
    echo "INCLUDES=$INCLUDES" >> $mf
    echo "" >> $mf
    echo "all: $BINFILE" >> $mf

    objlist=""
    for sfile in $SOURCE_FILES ; do
        objlist="$objlist ${sfile/%.c/.o}"
    done

    echo "" >> $mf
    echo "$BINFILE: $objlist" >> $mf
    echo -e "\t\$(CC) $objlist \$(LIBS) -o $BINFILE" >> $mf
    echo >> $mf

    for sfile in $SOURCE_FILES ; do
        ${CC} -MM $sfile >> $mf
        echo -e "\t\$(CC) -c \$(CFLAGS) \$(INCLUDES) $sfile -o ${sfile/%.c/.o}" >> $mf
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
    echo "" >> $mf

    echo "distclean: clean" >> $mf
    echo -e "\t-rm -f $GENERATED_HEADERS" >> $mf
    echo -e "\t-rm -f Makefile" >> $mf
    echo "" >> $mf

    echo ".PHONY: all clean distclean" >> $mf
    echo "" >> $mf

    cd $topdir

    return 0
}


if ! $(have_program pkg-config) ; then
    echo "Can't find pkg-config, aborting...">&2
    exit 4
fi

if ! $(have_ncursesw) ; then
    echo "Can't find ncursesw, aborting...">&2
    exit 1
else
    echo "Found ncursesw"
    get_ncurses_flags
fi

if [ $DEBUG -eq 0 ] ; then
    echo "Debug is enabled"
else
    echo "Debug is disabled"
fi

echo -n "Creating $srcdir/config.h..."
if $(write_config_h) ; then
    echo -e "\tOk"
else
    echo -e "\tFailed! Aborting..." >&2
    exit 2
fi

echo -n "Creating Makefile... "
if $(create_makefile) ; then
    echo -e "\t\tOk"
else
    echo -e "\tFailed! Aborting..." >&2
    exit 3
fi

echo -n "Creating $srcdir/Makefile... "
if $(create_src_makefile) ; then
    echo -e "\tOk"
else
    echo -e "\tFailed! Aborting..." >&2
    exit 3
fi

