#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="mate-control-center"

(test -f $srcdir/configure.ac) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

which mate-autogen || {
    echo "You need to install mate-common"
    exit 1
}

which yelp-build || {
    echo "You need to install yelp-tools"
}

USE_COMMON_DOC_BUILD=yes

. mate-autogen

