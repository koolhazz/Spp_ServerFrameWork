#!/bin/bash
ps aux|grep spp_ctrl|grep frame_|grep -v grep|awk '{printf $2"\t"}' >reload_pid.txt
cat >reload_pid.txt|xargs kill -35
rm -f >reload_pid.txt

