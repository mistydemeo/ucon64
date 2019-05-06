#! /bin/bash
# Yes, Bash, because we use features specific to it.

# Make it possible to specify another location (DESTDIR=/usr/bin ./install.sh).
if [ -z "$DESTDIR" ]; then
DESTDIR=/usr/local/bin
fi
if [ ! -e "$DESTDIR" ]; then
if [ ! "$DESTDIR" == "/usr/local/bin" ]; then
echo "$DESTDIR does not exist, installing to /usr/local/bin"
fi
DESTDIR=/usr/local/bin
if [ ! -e "$DESTDIR" ]; then
echo "/usr/local/bin does not exist, trying to create it"
mkdir -p "$DESTDIR"
fi
fi

if [ $OSTYPE == cygwin -o $OSTYPE == msys ]; then
cp -p ucon64 "$DESTDIR"
else
echo "Give root's password:"
# The version of su on Mac OS X requires the user name to be specified
su root -c "
echo Continuing installation.
chown root ucon64
chmod 4755 ucon64
cp -p ucon64 \"$DESTDIR\"
"
fi
if [ ! -e "$HOME/.ucon64" ]; then
mkdir "$HOME/.ucon64"
fi
if [ ! -e "$HOME/.ucon64/dat" ]; then
mkdir "$HOME/.ucon64/dat"
echo "You can copy/move your DAT file collection to $HOME/.ucon64/dat"
fi

if [ ${OSTYPE:0:6} == darwin ]; then
LIBSUFFIX=.dylib
elif [ $OSTYPE == cygwin ]; then
LIBSUFFIX=.dll
elif [ $OSTYPE == msys ]; then
LIBSUFFIX=.dll
else
LIBSUFFIX=.so
fi

if [ -f libdiscmage/discmage$LIBSUFFIX ]; then
cp libdiscmage/discmage$LIBSUFFIX "$HOME/.ucon64"
elif [ -f discmage$LIBSUFFIX ]; then
cp discmage$LIBSUFFIX "$HOME/.ucon64"
fi
echo "Be sure to check $HOME/.ucon64rc for some options after"
echo "you have run uCON64 once."
echo
