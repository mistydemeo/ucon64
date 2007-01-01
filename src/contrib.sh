#!/bin/sh
IN=../contrib
OUT=../src/ucon64_contrib.h
cd $IN
./bin2c.sh f2afirm.hex  >$OUT
./bin2c.sh iclientp.bin >>$OUT
./bin2c.sh iclientu.bin >>$OUT
./bin2c.sh ilogo.bin    >>$OUT
./bin2c.sh loader.bin   >>$OUT
./bin2c.sh sc_menu.bin  >>$OUT
./txt2c.sh genpal.txt   >>$OUT
./txt2c.sh mdntsc.txt   >>$OUT
./txt2c.sh snescopy.txt >>$OUT
./txt2c.sh snesntsc.txt >>$OUT
./txt2c.sh snespal.txt  >>$OUT
./txt2c.sh snesslow.txt >>$OUT
