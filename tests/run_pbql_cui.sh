#!/bin/sh -e
#
# This script is used to run the pbql console user interface along with
# all the dependencies:
#
#  * the proxy
#  * daemon
#  * communicatord
#  * fluid settings
#

LOG_FILE="tmp/cui.log"

if ! test -f tests/run_pbql_cui.sh
then
	echo "error: script must be started from the top folder of the prinbee project."
	exit 1;
fi

# Create a temporary folder if it does not exist yet
#
mkdir -p tmp

# Remove the previous log file (that way we have one session in the entire
# file which makes it easier to follow)
#
rm -f "${LOG_FILE}"

# Recompile so we run the latest
#
./mk

# Now run the pbql command
#
../../BUILD/Debug/contrib/prinbee/cui/pbql --log-file "${LOG_FILE}" --documentation ../../BUILD/Debug/dist/share/doc/prinbee/cui/ --trace

# Reset the TTY, just in case
#
#sane-tty
