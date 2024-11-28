#!/bin/bash

set -e

# Create emulated serial devices
CSP_SERVER_SERIAL_DEV=/tmp/ttyUSB0
socat -d -d -d pty,raw,echo=0,link=${CSP_SERVER_SERIAL_DEV} pty,raw,echo=0,link=/tmp/ttyUSB1 &
sleep 1
$1 -d ${CSP_SERVER_SERIAL_DEV} -p uart -a 10 -v 0 --csp_debug &
CSP_SERVER_TEST_PID=$!

# Run the server for 2 second
TIMEOUT=2
sleep $TIMEOUT

# Send sigterm to the server
kill -s SIGTERM  $CSP_SERVER_TEST_PID
pkill socat
