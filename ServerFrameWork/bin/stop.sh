#!/bin/bash
ps aux|grep frame_|grep -v grep|awk '{printf $2"\t"}' >pid.txt
cat pid.txt|xargs kill -10
rm -f pid.txt
