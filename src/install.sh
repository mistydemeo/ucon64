#!/bin/sh
CONFIG_DIR=~/.ucon64

echo Give root\'s password:
su -c "
echo Continueing installation.
chown root ucon64
chmod 4775 ucon64
cp ucon64 /usr/local/bin
"
if test ! -e $CONFIG_DIR; then
  mkdir $CONFIG_DIR
fi
cp libdiscmage/libdiscmage.so $CONFIG_DIR
