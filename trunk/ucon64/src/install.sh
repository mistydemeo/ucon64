#!/bin/sh

echo Give root\'s password:
su -c "
echo Continueing installation.
chown root ucon64
chmod 4775 ucon64
cp ucon64 /usr/local/bin
"
if ! test -x $HOME/.ucon64; then
mkdir $HOME/.ucon64
fi
if ! test -x $HOME/.ucon64/dat; then
mkdir $HOME/.ucon64/dat
echo "You can copy/move your DAT file collection to $HOME/.ucon64/dat"
fi
cp libdiscmage/discmage.so $HOME/.ucon64
echo "Be sure to check $HOME/.ucon64rc for some options"
echo
