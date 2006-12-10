#!/bin/sh
echo "#define $1 \\"
 
sed \
	-e 's:\\:\\\\:g' \
	-e "s:':\\\':g" \
	-e 's:":\\":g' \
 	-e 's:^:  ":'    \
	-e 's:$:\\n" \\:'
 
echo '  ""'
