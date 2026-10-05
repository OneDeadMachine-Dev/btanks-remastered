#!/bin/sh
VER=`svnversion -cn trunk`
REV=`echo "$VER" | cut -d: -f 2`
VER_LONG="0.9.$REV"
DIR="btanks-$VER_LONG"
echo "revision $REV"
echo "cleaning up..."
rm -rf $DIR
echo "exporting..."
svn export trunk $DIR
echo -n $VER > $DIR/.svnversion
rm -f $DIR/data/maps/test*
echo "packing..."
tar cjf btanks-$VER_LONG.tar.bz2 btanks-$VER_LONG
echo "done..."
