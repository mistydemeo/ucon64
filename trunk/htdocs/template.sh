#!/bin/sh
#rsstool "rss.slashdot.org/slashdot/slashdotgames" --template=index.template -r >index.htm
rsstool "rss.slashdot.org/slashdot/slashdotgames" \
 "http://news.google.com/news?hl=en&ned=us&q=wii,+nintendo,+playstation,+ps3,+xbox&ie=UTF-8&output=rss" \
 --template=index.template -r >index.htm
ln -sf index.htm index.html
ln -sf index.htm index2.htm
ln -sf index.htm index2.html
