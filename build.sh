#!/bin/bash
if [[ "$@" == "--help" ]]; then
	echo "Usage: ./build.sh"
	echo "Arguments:"
	echo "    --debug: compile with logging information"
	echo "    --help: display this message"
	exit
fi

if [[ "$@" == "--debug" ]]; then
	gcc main.c -o main -w -lm -DLOGGING_ENABLED -O2
	echo "Built with debug information."
else
	gcc main.c -o main -w -lm -O2
	echo "Built without debug information."
fi
