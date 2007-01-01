#!/bin/sh
echo "const unsigned char `echo $1|sed -e 's:\.:_:g' -e 'y:ABCDEFGHIJKLMNOPQRSTUVWXYZ:abcdefghijklmnopqrstuvwxyz:'`[`du -b $1|sed -e "s:$1::"`] = {"
cat $1|hexdump -v -e '"  " 8/1 "0x%02x, " "\n"'|sed -e 's: 0x  ,::g'
echo "};"
