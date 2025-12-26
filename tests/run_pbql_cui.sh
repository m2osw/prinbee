#!/bin/sh -e
#
# This script is used to run the pbql console user interface along with
# all the dependencies:
#
#  * the proxy
#  * daemon
#  * communicatord
#  * fluid settings
#  * cluck
#

# TODO: move those to the BUILD folder
#
CUI_LOG_FILE="tmp/cui.log"
COMMUNICATORD_LOG_FILE="tmp/communicatord.log"
DAEMON_LOG_FILE="tmp/daemon.log"
FLUID_SETTINGS_LOG_FILE="tmp/fluid-settings.log"
PROXY_LOG_FILE="tmp/proxy.log"
SIGNAL_LOG_FILE="tmp/signal.log"
COMMUNICATORD_SOCK="tmp/communicatord.sock"

if ! test -f tests/run_pbql_cui.sh
then
	echo "error: script must be started from the top folder of the prinbee project."
	exit 1;
fi

# Create a temporary folder if it does not exist yet
# and a sub-directory "contexts" for the prinbee daemon to use to create files
#
mkdir -p tmp/contexts

# Remove the previous log file (that way we have one session in the entire
# file which makes it easier to follow)
#
rm -f "${CUI_LOG_FILE}" \
	"${COMMUNICATORD_LOG_FILE}" \
	"${DAEMON_LOG_FILE}" \
	"${FLUID_SETTINGS_LOG_FILE}" \
	"${PROXY_LOG_FILE}"

# Recompile so we run the latest
#
./mk -i

# Start the communicator daemon
#
echo "--- start communicator daemon"
ADVGETOPT_OPTIONS_FILES_DIRECTORY="../../BUILD/Debug/dist/share/communicator/options" ../../BUILD/Debug/contrib/communicator/daemon/communicatord \
	--log-file "${COMMUNICATORD_LOG_FILE}" \
	--trace \
	--debug-all-messages \
	--my-address 127.0.0.1 \
	--services ../BUILD/Debug/dist/share/communicator/services \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--timedate-wait-command "../../BUILD/Debug/dist/bin/timedate-wait" \
	--unix-listen "${COMMUNICATORD_SOCK}" &

# Start the fluid-settings daemon
#
echo "--- start fluid-settings"
../../BUILD/Debug/contrib/fluid-settings/daemon/fluid-settings \
	--log-file "${FLUID_SETTINGS_LOG_FILE}" \
	--trace \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--communicatord-listen "cd://`pwd`/${COMMUNICATORD_SOCK}" &

# Start the proxy daemon
#
echo "--- start prinbee proxy"
../../BUILD/Debug/contrib/prinbee/proxy/prinbee-proxy \
	--log-file "${PROXY_LOG_FILE}" \
	--trace \
	--owner alexis:alexis \
	--prinbee-path "`pwd`/tmp" \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--cluster-name "test_cluster" \
	--communicatord-listen "cd://`pwd`/${COMMUNICATORD_SOCK}" &

# Start the actual daemon
#
echo "--- start prinbee daemon"
../../BUILD/Debug/contrib/prinbee/daemon/prinbee-daemon \
	--log-file "${DAEMON_LOG_FILE}" \
	--trace \
	--owner alexis:alexis \
	--prinbee-path "`pwd`/tmp" \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--cluster-name "test_cluster" \
	--communicatord-listen "cd://`pwd`/${COMMUNICATORD_SOCK}" &

# Now run the pbql command
#
echo "--- start pbql..."
../../BUILD/Debug/contrib/prinbee/cui/pbql \
	--log-file "${CUI_LOG_FILE}" \
	--trace \
	--documentation "../../BUILD/Debug/dist/share/doc/prinbee/cui/" \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--communicatord-listen "cd://`pwd`/${COMMUNICATORD_SOCK}"

# Stop everything except the communicator daemon itself
echo "--- ed-signal to broadcast STOP to all clients..."
../../BUILD/Debug/contrib/eventdispatcher/tools/ed-signal \
	--console \
	--trace \
	--except-stack-collect complete \
	./STOP

# Ask the communicator daemon to shutdown
#
#sleep 1
../../BUILD/Debug/contrib/eventdispatcher/tools/ed-stop \
	--process-name \
	--service communicatord

# Make sure it all ends
#
echo "info: waiting for background services to exit"
wait

# If necessary, reset the TTY, just in case
#
#sane-tty
