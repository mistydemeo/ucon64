#!/bin/sh

echo Give root\'s password:
su -c "
echo Continueing installation.
chown root ucon64
chmod 4775 ucon64
cp ucon64 /usr/local/bin
if [ -e "$HOME/.ucon64rc" ]
then
	cp $HOME/.ucon64rc $HOME/.ucon64rc.old
fi
cp .ucon64rc $HOME
"