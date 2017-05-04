#!/bin/bash

CLIENT_IP=$1

echo "Mail generated: $(date)"
echo "==== Authentication success ===="
echo "Authentication succeeded for ip: $CLIENT_IP."
echo "It is now given 'secure' shell access to the unit configured."
echo "==== Unit IP ===="
echo "$(ifconfig)"
