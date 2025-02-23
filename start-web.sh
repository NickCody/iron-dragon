#!/bin/bash

set -euo pipefail

mkdir -p www/cgi-bin

if [ -x build/cgi_id.exe ]; then
    echo "Deploying new cgi_id.exe binary to cgi-bin"
    cp build/cgi_id.exe www/cgi-bin
fi

echo "Starting web server"
python3 cgi_server.py
