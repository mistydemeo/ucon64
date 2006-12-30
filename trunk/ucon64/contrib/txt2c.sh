#!/bin/sh
echo "#define $1 \\"
 
sed $1 \
	-e 's:\\:\\\\:g' \
	-e "s:':\\\':g" \
	-e 's:":\\":g' \
 	-e 's:^:  ":'    \
	-e 's:$:\\n" \\:'
 
echo '  ""'
