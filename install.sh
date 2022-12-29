#!/bin/bash
gcc *.c libtmsolve/*.c -O2 -lreadline -lm -D LOCAL_BUILD -o tmsolve
mv tmsolve /usr/bin
# SELinux context probably incorrect if build happened in the home dir.
restorecon /usr/bin/tmsolve
echo Done!