 hexdump -v -e '"" 16/1 "0x%02x, " "\n"'


#!/bin/sh
echo "const unsigned char $1[] = {"
 
sed \
	-e 's:\\:\\\\:g' \
	-e "s:':\\\':g" \
	-e 's:":\\":g' \
 	-e 's:^:  ":'    \
	-e 's:$:\\n" \\:'
 
echo '  ""'
