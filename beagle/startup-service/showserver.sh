#!/bin/sh
SLEEPSEC=2
MAXLOOP=35
loopcount=0
PRUREADY=0

sleep  50
/root/startup/ShowServer /media/server.cfg 1>/dev/null 2>/dev/null

FILE_PATH="/media/showshutdown.sh"

if [ -f "$FILE_PATH" ]; then

    shutdown -h now

fi
