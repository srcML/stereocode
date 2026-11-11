#!/bin/sh
# Runs a command of the image as the owner of the mounted folder /data,
# which keeps the user of a Linux host as the owner of the output files.
set -eu

uid=$(stat -c %u /data)
gid=$(stat -c %g /data)
if [ "$(id -u)" -eq 0 ] && [ "$uid" -ne 0 ]; then
    exec setpriv --reuid="$uid" --regid="$gid" --clear-groups "$@"
fi
exec "$@"
