#!/bin/bash

set -euo pipefail

# Start idpcdu daemon
build/idpcdu -f config/idpcd.config
