#!/bin/bash

set -euo pipefail

mkdir -p www/cgi-bin

if [ -x build/cgi_id.exe ]; then
    echo "Deploying new cgi_id.exe binary to cgi-bin"
    cp build/cgi_id.exe www/cgi-bin
fi

# Kill previous daemon

pkill idpcdu || echo "No previous daemon found"

# Start idpcdu daemon
build/idpcdu -f config/idpcd.config
echo "Started idpcdu"


echo "Starting web server"
python3 cgi_server.py
