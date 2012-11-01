#!/usr/bin/env sh

srcdir="src"
VERSION=$(cat "${srcdir}/VERSION")
CONFIG_HEADER="${srcdir}/config.h"

# To enable debug, set this to zero
DEBUG=1

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
