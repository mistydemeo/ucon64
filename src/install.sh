#!/bin/sh

echo Give root\'s password:
su -c "
echo Continuing installation.
chown root ucon64
chmod 4775 ucon64
cp ucon64 /usr/local/bin
"
