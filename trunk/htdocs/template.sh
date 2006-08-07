#!/bin/sh
#http://www.youtube.com/rss/tag/snes.rss
#rsstool "rss.slashdot.org/slashdot/slashdotgames" --template=index.template -r >index.htm
#rsstool "rss.slashdot.org/slashdot/slashdotgames" \
# "http://news.google.com/news?hl=en&ned=us&q=wii,+nintendo,+playstation,+ps3,+xbox&ie=UTF-8&output=rss" \
# --template=index.template -r >index.htm
rsstool --template=index.template -r \
"http://www.youtube.com/rss/tag/snes.rss" \
"http://www.youtube.com/rss/tag/gameboy.rss" \
"http://www.youtube.com/rss/tag/nintendo.rss" \
"http://www.youtube.com/rss/tag/genesis.rss" \
"http://www.youtube.com/rss/tag/nes.rss" \
"http://www.youtube.com/rss/tag/c64.rss" \
>index.htm
ln -sf index.htm index.html
ln -sf index.htm index2.htm
ln -sf index.htm index2.html
