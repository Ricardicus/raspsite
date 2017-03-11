#!/bin/bash

BACKEND_PORT=$1

if [ ${#BACKEND_PORT} -eq 0 ]; then
	echo "Usage: $0 port"
	exit 0
fi

# Read keep-alive or not file
while [ -f "$HOME/coding/webb/raspsite/etc/keep-alive.txt" ]; do
	# Watchdog is activated
	# List the processes a search for backend
	BACKEND_PID=$(ps -a | grep backend | awk '{ print $1; }' )
	if [ ${#BACKEND_PID} -gt 0 ]; then
		# The pid exists
		while [ -e "/proc/$BACKEND_PID" ]; do
			# the process has not stopped
			sleep 5
		done

		# The backend process has stopped, need to reinitialize it!
		# Waiting for socket timeouts.
		sleep 60
		
		# Read keep-alive or not file
		./backend $BACKEND_PORT # launch it

		sleep 4 # relaxing the watchdog

	else 
		echo "Watchdog: Could not find 'backend' among the active processes, will start it."
		./backend BACKEND_PORT
		sleep 60 # Waiting for sockets to timeout
	fi	
done

echo "Watchdog session ended"
