#!/bin/sh
#
# ABSOLUTELY NO WARRANTY WITH THIS PACKAGE. USE IT AT YOUR OWN RISK.
#
# Download, re-config & build latest mutt-ng tarball as RPM & SRPM files
#
# Usage: muttng-rpmbuild.sh
#
# Copyright (C) 2005  muttng-rpmbuild.sh 1.10  Iain Lea  iain@bricbrac.de
#
#   This program is free software; you can redistribute it and/or 
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation; either version 2 of
#   the License, or (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful, 
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
#   See the GNU General Public License for more details.
# 
#   You should have received a copy of the GNU GPL along with this
#   program; if not, write to the Free Software Foundation, Inc.,
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# Requirements:
#   ...
#
# Contributors:
#   ...
#
# Changelog:
#   1.10
#   - added cosmetic changes concerning sent mail
#   - added rm -rf $SRCDIR/muttng-*  and more cleanup on exit
#   1.00
#   - initial release
#
# Todo:
#   - add section of Changelog/SVN to sent mail if successful

# Local site specific variables. Change to suit your site!
ADDR=root
MAIL=mutt
ARCH=i386
TMPDIR=/tmp
DSTDIR=/var/www/iainlea.dyndns.org/software/muttng
BLDDIR=/usr/src/redhat
#
# Remote site specific variables. Should not need changing!
SITE=http://mutt-ng.berlios.de/snapshots
FILE=muttng-latest.tar.gz
#
# You should not need to change anything below this line!
SRCDIR=$BLDDIR/SOURCES
RPMSDIR=$BLDDIR/RPMS/$ARCH
SRPMDIR=$BLDDIR/SRPMS
SPEC=muttng.spec

# Sanity checks
if [ ! -d $TMPDIR ]; then
	echo "Error: \$TMPDIR $TMPDIR - does not exist. Edit to match your site!"
	exit 0
fi
if [ ! -d $DSTDIR ]; then
	echo "Error: \$DSTDIR $DSTDIR - does not exist. Edit to match your site!"
	exit 0
fi
if [ ! -d $SRCDIR ]; then
	echo "Error: \$SRCDIR $SRCDIR - does not exist. Edit to match your site!"
	exit 0
fi
if [ ! -d $RPMSDIR ]; then
	echo "Error: \$RPMSDIR $RPMSDIR - does not exist. Edit to match your site!"
	exit 0
fi
if [ ! -d $SRPMDIR ]; then
	echo "Error: \$SRPMDIR $SRPMDIR - does not exist. Edit to match your site!"
	exit 0
fi

cd $TMPDIR

echo "Backup:  $TMPDIR/$SPEC"
/bin/mv -f $TMPDIR/$SPEC $TMPDIR/$SPEC.bak >/dev/null 2>&1
/bin/rm -f $TMPDIR/$SPEC.in >/dev/null 2>&1

echo "Delete:  $TMPDIR/$FILE"
/bin/rm -f $TMPDIR/$FILE >/dev/null 2>&1

echo "Delete:  $TMPDIR/muttng-*/"
(find ./muttng-* -type d -print | sort -r | cut -d '/' -f2 | uniq | xargs /bin/rm -rf) >/dev/null 2>&1

echo "Delete:  $SRCDIR/muttng-*.tar.gz"
(find $SRCDIR/muttng-*tar.gz -print | sort -r | xargs /bin/rm -f) >/dev/null 2>&1

echo "Delete:  $SRCDIR/muttng-*/"
# find $SRCDIR/muttng-* -type d -print | sort -r | cut -d '/' -f2 | uniq | xargs /bin/rm -rf >/dev/null 2>&1
(cd $SRCDIR; find ./muttng-* -type d -print | sort -r | cut -d '/' -f2 | uniq | xargs /bin/rm -rf) >/dev/null 2>&1

echo "FetchIt  $SITE/$FILE"
wget -O $FILE $SITE/$FILE >/dev/null 2>&1

echo "Extract  $FILE"
tar xzf $FILE

NAME=`find ./muttng-* -type d -print | sort -r | cut -d '/' -f2 | uniq | head -1`
echo "TarBall  $NAME"

VER=`echo $NAME | cut -d '-' -f2`
# echo "Version  $VER"

echo "Parsing  $NAME/contrib/$SPEC"
cp $NAME/contrib/$SPEC $SPEC.in
sed -e "s/^Version:.*$/Version: $VER/" < $SPEC.in > $SPEC
REL=`egrep "Release: " $SPEC | cut -d ' ' -f2`
egrep "Version: " $SPEC
egrep "Release: " $SPEC

echo "BuildIt  $SRCDIR/$NAME.tar.gz"
mv $FILE $SRCDIR/$NAME.tar.gz
# sleep 3
rpmbuild -ba $SPEC >$TMPDIR/$NAME.rpmbuild 2>&1

# echo "CheckIt  $RPMSDIR/muttng-$VER-$REL.i386.rpm"
if [ -e $RPMSDIR/muttng-$VER-$REL.i386.rpm ]
then
	echo "BuiltOK  $RPMSDIR/muttng-$VER-$REL.i386.rpm"
	cp $RPMSDIR/muttng-$VER-$REL.i386.rpm $DSTDIR
	echo "$DSTDIR/muttng-$VER-$REL.i386.rpm" > $TMPDIR/$NAME.rpmbuild

#	echo "CheckIt  $SRPMDIR/muttng-$VER-$REL.src.rpm"
	if [ -e $SRPMDIR/muttng-$VER-$REL.src.rpm ]
	then
		echo "BuiltOK  $SRPMDIR/muttng-$VER-$REL.src.rpm"
		cp $SRPMDIR/muttng-$VER-$REL.src.rpm $DSTDIR
		echo "$DSTDIR/muttng-$VER-$REL.src.rpm" >> $TMPDIR/$NAME.rpmbuild
	fi
	
	cp $SPEC $DSTDIR

	echo "Mailing  $ADDR - BUILT"
	echo >> $TMPDIR/$NAME.rpmbuild
	head -15 $TMPDIR/$NAME/ChangeLog.mutt-ng >> $TMPDIR/$NAME.rpmbuild
	echo "..." >> $TMPDIR/$NAME.rpmbuild
	$MAIL -s "Mutt-NG: BUILT - $NAME" $ADDR < $TMPDIR/$NAME.rpmbuild
else
	echo "Mailing  $ADDR - ERROR"
	$MAIL -s "Mutt-NG: ERROR - $NAME" $ADDR < $TMPDIR/$NAME.rpmbuild
fi

# echo "Cleanup  $TMPDIR/muttng-*/ $TMPDIR/$NAME.rpmbuild"
/bin/rm -f $TMPDIR/$NAME.rpmbuild
(cd $TMPDIR; find ./muttng-* -type d -print | sort -r | cut -d '/' -f2 | uniq | xargs /bin/rm -rf) >/dev/null 2>&1

exit 1
