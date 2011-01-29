#!/bin/bash

TYPE=$1
SRCDIR=$2

usage()
{
	echo "$0 [nightly|stable] srcdir"
	echo "set environment FORCE to 1 to force update of archive"
	exit 1
}

cleanup()
{
	echo -n ""
	#rm -rf /tmp/$$
}

error()
{
	cleanup
	exit 1
}

if [ -z "$TYPE" ] ; then
	usage
fi

if [ ! -d "$SRCDIR" ] ; then
	usage
fi

RELEASEDEF=$SRCDIR/release/release.$TYPE.def
source $RELEASEDEF || error

if [ ! -d "$TARGET" ] ; then
	echo "WARN: $RELEASEDEF does not contain TARGET, using $(pwd)"
	TARGET=$(pwd)
fi

# get version
VERSION=$(grep WINDOW_VERSION build_version.h | cut -d ' ' -f 3 | cut -d \" -f 2)

# get revision
REVISION=$(grep WINDOW_REVISION build_version.h | cut -d ' ' -f 3 | cut -d \" -f 2)

# get arch
ARCH="$(grep COMPILEFOR CMakeCache.txt | cut -d '=' -f 2 | head -n 1).$(grep COMPILEARCH CMakeCache.txt | cut -d '=' -f 2 | head -n 1)"

# current and new package directory
ARCHDIR=$TARGET/$ARCH
ARCHNEWDIR=$TARGET/$ARCH.new

rm -rf $ARCHNEWDIR
mkdir -p $ARCHNEWDIR
mkdir -p $ARCHNEWDIR/packed
mkdir -p $ARCHNEWDIR/unpacked
mkdir -p $ARCHNEWDIR/updater

# redirect output to log AND stdout
npipe=/tmp/$$.log
trap "rm -f $npipe" EXIT
mknod $npipe p
tee <$npipe $ARCHNEWDIR/build.log &
exec 1>&-
exec 1>$npipe

echo "Building $TYPE for $VERSION-$REVISION/$ARCH in $SRCDIR"

make || error

if [ $REVISION -eq 0 ] ; then
	# get revision again
	REVISION=$(grep WINDOW_REVISION build_version.h | cut -d ' ' -f 3 | cut -d \" -f 2)
fi

if [ $REVISION -eq 0 ] ; then
	echo "error: revision is null"
	error
fi

DESTDIR=$ARCHNEWDIR/unpacked/s25rttr_$VERSION
make install DESTDIR=$DESTDIR || error

# do they differ?
CHANGED=1
if [ ! "$FORCE" = "1" ] && [ -d $ARCHDIR/unpacked/s25rttr_$VERSION ] ; then
	diff -qrN $ARCHDIR/unpacked/s25rttr_$VERSION $DESTDIR
	CHANGED=$?
fi

if [ "$FORCE" = "1" ] ; then
	echo "FORCE is set - forcing update"
fi

# create packed data and updater
if [ $CHANGED -eq 1 ] || [ ! -f $ARCHDIR/packed/s25rttr.tar.bz2 ] ; then
	echo "creating new archive"
	
	# pack
	tar -C $ARCHNEWDIR/unpacked \
		--exclude=.svn \
		--exclude s25rttr_$VERSION/share/s25rttr/RTTR/MUSIC/SNG/SNG_*.OGG \
		--exclude s25rttr_$VERSION/RTTR/MUSIC/SNG/SNG_*.OGG \
		--exclude s25rttr_$VERSION/s25client.app/Contents/MacOS/share/s25rttr/RTTR/MUSIC/SNG/SNG_*.OGG \
		-cvjf $ARCHNEWDIR/packed/s25rttr.tar.bz2 s25rttr_$VERSION || error
	
	# link to archive
	mkdir -p $ARCHIVE
	ln -v $ARCHNEWDIR/packed/s25rttr.tar.bz2 $ARCHIVE/s25rttr_$VERSION-${REVISION}_$ARCH.tar.bz2

	# do upload
	if [ ! -z "$UPLOADTARGET" ] ; then
		scp $ARCHIVE/s25rttr_$VERSION-${REVISION}_$ARCH.tar.bz2 $UPLOADTARGET$UPLOADTO
		if [ ! -z "$UPLOADTARGET" ] ; then
			echo "$UPLOADURL$(basename $file)" >> ${UPLOADFILE}rapidshare.txt
		fi
	fi
	
	echo "creating new updater tree"

	# fastcopy files (only dirs and files, no symlinks
	(cd $ARCHNEWDIR/unpacked/s25rttr_$VERSION && find -type d -exec mkdir -vp $ARCHNEWDIR/updater/{} \;)
	(cd $ARCHNEWDIR/unpacked/s25rttr_$VERSION && find -type f -exec cp {} $ARCHNEWDIR/updater/{} \;)
	
	# note symlinks
	echo -n > /tmp/links.$$
	(cd $ARCHNEWDIR/unpacked/s25rttr_$VERSION && find -type l -exec bash -c 'echo "{} $(readlink {})" >> /tmp/links.$$' \;)
	
	# note hashes
	(cd $ARCHNEWDIR/updater && md5deep -r -l . > /tmp/files.$$)

	# bzip files
	find $ARCHNEWDIR/updater -type f -exec bzip2 -v {} \;
	
	# move file lists
	mv /tmp/links.$$ $ARCHNEWDIR/updater/links
	mv /tmp/files.$$ $ARCHNEWDIR/updater/files

	# create human version notifier
	touch $ARCHNEWDIR/revision-${REVISION}
	touch $ARCHNEWDIR/version-$VERSION
	
	# rotate trees
	rm -rf $ARCHDIR.5
	mv $ARCHDIR.4 $ARCHDIR.5
	mv $ARCHDIR.3 $ARCHDIR.4
	mv $ARCHDIR.2 $ARCHDIR.3
	mv $ARCHDIR.1 $ARCHDIR.2
	mv $ARCHDIR $ARCHDIR.1
	mv $ARCHNEWDIR $ARCHDIR

	echo "done"
else
	echo "nothing changed - no update necessary"
fi

cleanup
exit 0
