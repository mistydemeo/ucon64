#!/bin/sh
DEST=/usr/local/bin

echo Give root\'s password:
su -c "
echo Continueing installation.
chown root ucon64
chmod 4775 ucon64
cp ucon64 $DEST
"
if [ ! -e $HOME/.ucon64 ]; then
mkdir $HOME/.ucon64
fi
if [ ! -e $HOME/.ucon64/skin ]; then
mkdir $HOME/.ucon64/skin
cp skin/*.png $HOME/.ucon64/skin
fi
if [ ! -e $HOME/.ucon64/dat ]; then
mkdir $HOME/.ucon64/dat
echo "You can copy/move your DAT file collection to $HOME/.ucon64/dat"
fi
if [ -f libdiscmage/discmage.so ]; then
cp libdiscmage/discmage.so $HOME/.ucon64
elif [ -f discmage.so ]; then
cp discmage.so $HOME/.ucon64
fi
if [ -f libnetgui/netgui.so ]; then
cp libnetgui/netgui.so $HOME/.ucon64
ln -sf $DEST/ucon64 $DEST/gucon64
ln -sf $DEST/ucon64 $DEST/ucon64gui
elif [ -f netgui.so ]; then
cp netgui.so $HOME/.ucon64
ln -sf $DEST/ucon64 $DEST/gucon64
ln -sf $DEST/ucon64 $DEST/ucon64gui
fi
echo "Be sure to check $HOME/.ucon64rc for some options after"
echo "you've run uCON64 once."
echo
