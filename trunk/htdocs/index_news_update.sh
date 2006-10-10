SEARCH1="nintendo+emulation"
SEARCH2="gp2x+emulation"
SEARCH3="sega+emulation"
SEARCH4="sony+emulation"
SEARCH5="play+station+emulation"
SEARCH6="ps2+emulation"
SEARCH7="dreamcast+emulation"
SEARCH8="snes+emulation"
rsstool --template=index_news.html.inc.in -r \
"http://www.youtube.com/rss/tag/"$SEARCH1".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH2".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH3".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH4".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH5".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH6".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH7".rss" \
"http://www.youtube.com/rss/tag/"$SEARCH8".rss" \
>index_news.html.inc
scp index_news.html.inc noisyb@ucon64.sf.net:/home/groups/u/uc/ucon64/htdocs
