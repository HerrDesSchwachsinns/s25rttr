#!/bin/sh

TOP_SRCDIR=$1
SRCDIR=$PWD

if (test "${TOP_SRCDIR}err" = "err") || (test "${TOP_SRCDIR}" = ".") ; then
	TOP_SRCDIR=$SRCDIR
fi

echo "Cleaning up $PWD"
rm -vf CMakeCache.txt cmake_install.cmake Makefile install_manifest.txt
rm -vrf CMakeFiles
rm -vf *.bak *~ .DS_Store

# Unterverzeichnisse entfernen
for I in `grep ADD_SUBDIRECTORY ../CMakeLists.txt | cut -d '(' -f 2 \
          | cut -d ')' -f 1` ; do
	rm -vrf "$I"
done

exit 0
