#!/bin/sh

stty -F /dev/ttyUSB0 115200
#115200
while [ 1 ]
do
cat /dev/ttyUSB0
done
