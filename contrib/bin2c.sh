#!/bin/sh
echo "const unsigned char $1[] = {"
hexdump -v -e '"  " 8/1 "0x%02x, " "\n"' $1
echo "};"
