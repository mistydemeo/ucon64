SEARCH1="nintendo+emulation"
SEARCH2="gp2x+emulation"
SEARCH3="sega+emulation"
SEARCH4="sony+emulation"
SEARCH5="play+station+emulation"
SEARCH6="ps2+emulation"
SEARCH7="dreamcast+emulation"
SEARCH8="snes+emulation"
rsstool --template=index.template -r \
"http://www.youtube.com/rss/tag/"$SEARCH1".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH2".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH3".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH4".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH5".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH6".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH7".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH8".rss" \
"http://video.google.com/videosearch?q="$SEARCH1"+is%3Afree&page=1&so=1&output=rss" \
"http://video.google.com/videosearch?q="$SEARCH2"+is%3Afree&page=1&so=1&output=rss" \
"http://video.google.com/videosearch?q="$SEARCH3"+is%3Afree&page=1&so=1&output=rss" \
"http://video.google.com/videosearch?q="$SEARCH4"+is%3Afree&page=1&so=1&output=rss" \
"http://video.google.com/videosearch?q="$SEARCH5"+is%3Afree&page=1&so=1&output=rss" \
"http://video.google.com/videosearch?q="$SEARCH6"+is%3Afree&page=1&so=1&output=rss" \
"http://video.google.com/videosearch?q="$SEARCH7"+is%3Afree&page=1&so=1&output=rss" \
"http://video.google.com/videosearch?q="$SEARCH8"+is%3Afree&page=1&so=1&output=rss" \
>index.htm
ln -sf index.htm index.html
ln -sf index.htm index2.htm
ln -sf index.htm index2.html
scp index.htm index.html index2.htm index2.html noisyb@ucon64.sf.net:/home/groups/u/uc/ucon64/htdocs
