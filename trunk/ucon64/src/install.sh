
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
cp libdiscmage/discmage.so $HOME/.ucon64
cp cache.zip $HOME/.ucon64
