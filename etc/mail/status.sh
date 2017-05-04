#!/bin/bash

echo "Mail generated: $(date)"
echo "==== Uptime ===="
echo "$(uptime)"
echo "==== Ps ===="
echo "$(ps -l)"
echo "==== VM status ===="
echo "$(vmstat)"