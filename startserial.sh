#!/bin/sh

echo 'Starting "socat" between ttyS0 and ttyS4...'

sudo socat PTY,link=/dev/ttyS0 PTY,link=/dev/ttyS4
