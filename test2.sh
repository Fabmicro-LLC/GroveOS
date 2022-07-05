#!/bin/sh

stty -F /dev/ttyUSB1 115200
#115200
while [ 1 ]
do
cat /dev/ttyUSB1
done
