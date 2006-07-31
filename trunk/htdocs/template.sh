#!/bin/sh
rsstool rss.slashdot.org/slashdot/slashdotgames --template=index.template -r >index.htm
ln -sf index.htm index.html
ln -sf index.htm index2.htm
ln -sf index.htm index2.html
