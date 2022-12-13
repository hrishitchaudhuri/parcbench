#!/bin/bash
OS_PATH='x86_64-linux-gnu/'
BIN_PATH='bin/'$OS_PATH

LOG_PATH='logs/threads/'
RD_TEST='../freqs.json'

OPTION=" open2close "


echo $OS_PATH
echo $BIN_PATH
echo $LOG_PATH
echo $RD_TEST
echo $OPTION

for ((c=10000000; c<=50000000; c+=10000000))
do
    echo "$c"
    LOG_NAME=$LOG_PATH$c'_1_128_o2c.txt'
    # echo $LOG_NAME
    BIN="par_rw -P 1 "
    echo $LOG_NAME
    echo $BIN_PATH$BIN$c$OPTION$RD_TEST
    pwd
    # time $BIN_PATH"par_rw -P 1 "$c" open2close "$RD_TEST
    # time $BIN_PATH$BIN$c
    { time $BIN_PATH$BIN$c$OPTION$RD_TEST ; } 2>&1 |tee $LOG_NAME
done