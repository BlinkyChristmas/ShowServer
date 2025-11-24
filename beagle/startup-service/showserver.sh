#!/bin/sh
/root/startup/ShowServer /media/server.cfg 1>/dev/null 2>/dev/null

FILE_PATH="/media/showshutdown.sh"

if [ -f "$FILE_PATH" ]; then

    shutdown -h now

fi
