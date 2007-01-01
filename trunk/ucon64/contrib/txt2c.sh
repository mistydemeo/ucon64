#!/bin/sh
echo "#define `echo $1|sed -e 's:\.:_:g' -e 'y:abcdefghijklmnopqrstuvwxyz:ABCDEFGHIJKLMNOPQRSTUVWXYZ:'` \\"
sed $1 \
-e 's:\\:\\\\:g' \
-e "s:':\\\':g" \
-e 's:":\\":g' \
-e 's:^:  ":' \
-e 's:$:\\n" \\:'
echo '  ""'
