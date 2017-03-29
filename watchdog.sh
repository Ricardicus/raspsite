#!/bin/bash

BACKEND_PORT=$1

if [ ${#BACKEND_PORT} -eq 0 ]; then
	echo "Usage: $0 port"
	exit 0
fi

# Read keep-alive or not file
while [ -f "$PWD/etc/keep-alive.txt" ]; do
	# Watchdog is activated
	# List the processes a search for backend
	ENTRY=$(ps -a | grep backend)
	BACKEND_PID=""
	while read p; do
		if ! echo $p | grep "grep" 1>/dev/null; then
			BACKEND_PID=$(echo "$p" | awk '{ print $1; }')
			break
		fi
	done <<< "$ENTRY"

	if [ ! ${#BACKEND_PID} -gt 0 ]; then
		echo "Watchdog: Could not find 'backend' among the active processes, will start it."
		./backend $BACKEND_PORT &
		sleep 10 # wait for the process to establish itself
		BACKEND_PID=$(ps -a | grep backend | awk '{ print $1; }' )
	fi

	if ! wait $BACKEND_PID 2>/dev/null; then
		# The backend process is not a child of this watchdog shell.
		# We will have to wait for the process to terminate by means of polling.
		if [ ${#BACKEND_PID} -gt 0 ]; then
			# The pid exists and its not a child of this shell

			if [ -e "/proc/$BACKEND_PID" ]; then
				while [ -e "/proc/$BACKEND_PID" ]; do # Will only work on linux
					# the process has not stopped
					sleep 5 # Checking every fifth second if the process is active
				done
			else 
				# We cannot wait the linux way... 
				# We kill the process and later reinitialize it
				echo "The backend is running on pid: $BACKEND_PID. This process is not a child of the watchdog and will be restarted."
				kill -9 $BACKEND_PID
				sleep 10
			fi

			# The backend process has stopped, need to reinitialize it!
			# Waiting for socket timeouts.

			echo "Watchdog: The backend process has stopped, waiting for socket timeouts..."
			sleep 60
			echo "Sockets timed out (most probably). Attempting to reinitialize the backend process."
			# Read keep-alive or not file
			./backend $BACKEND_PORT & # launch it in the background
			sleep 10 # wait for the process to establish itself
			BACKEND_PID=$(ps -a | grep backend | awk '{ print $1; }' )
		else 
			echo "Watchdog: Could not find 'backend' among the active processes, will start it."
			./backend $BACKEND_PORT &
			sleep 10 # wait for the process to establish itself
			BACKEND_PID=$(ps -a | grep backend | awk '{ print $1; }' )
		fi

	fi

done

echo "Watchdog session ended"
