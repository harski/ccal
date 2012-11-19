#!/usr/bin/env sh

build_aux_dir="build-aux"

# Create build-aux dir
if test ! -d "${build_aux_dir}"; then mkdir ${build_aux_dir}; fi

# Run the autotools
aclocal && autoconf && autoheader && automake --add-missing --copy

