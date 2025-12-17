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

CUI_LOG_FILE="tmp/cui.log"
COMMUNICATORD_LOG_FILE="tmp/communicatord.log"
COMMUNICATORD_SOCK="tmp/communicatord.sock"

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
rm -f "${CUI_LOG_FILE}" "${COMMUNICATORD_LOG_FILE}"

# Recompile so we run the latest
#
./mk

# Start the communicator daemon
#
../../BUILD/Debug/contrib/communicatord/daemon/communicatord \
	--log-file "${COMMUNICATORD_LOG_FILE}" \
	--trace \
	--my-address 127.0.0.1 \
	--services ../BUILD/Debug/dist/share/communicatord/services \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--timedate-wait-command "../../BUILD/Debug/dist/bin/timedate-wait" \
	--unix-listen "${COMMUNICATORD_SOCK}" &

# Now run the pbql command
#
../../BUILD/Debug/contrib/prinbee/cui/pbql \
	--log-file "${CUI_LOG_FILE}" \
	--trace \
	--documentation "../../BUILD/Debug/dist/share/doc/prinbee/cui/" \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--communicatord-listen "cd://`pwd`/${COMMUNICATORD_SOCK}"

# Ask the communicator daemon to shutdown
#
../../BUILD/Debug/contrib/eventdispatcher/tools/ed-stop \
	--process-name \
	--service communicatord

# Reset the TTY, just in case
#
#sane-tty
