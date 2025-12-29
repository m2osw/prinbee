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

COMMUNICATORD_SOCK="tmp/communicatord.sock"

if ! test -f tests/run_pbql_cui.sh
then
	echo "error: script must be started from the top folder of the prinbee project."
	exit 1;
fi

# Make sure daemons were stopped
#
check_daemon() {
	NAME=$1
	if pgrep -u "${USER}" "${NAME}" > /dev/null
	then
		echo "error: the \"${NAME}\" daemon is still running."
		echo
		echo "Do you want to kill it? (y/[N]) \c"
		read answer
		if test "${answer}" = "y" -o "${answer}" = "Y"
		then
			pkill -u "${USER}" "${NAME}"
		else
			echo "error: start process aborted."
			exit 1
		fi
		echo
	fi
}

check_daemon communicatord
check_daemon fluid-settings
check_daemon prinbee-proxy
check_daemon prinbee-daemon

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
# The PATH is changed so the systemctl defined in our tests folder is used
# and that way we can pretend that the ipload ran successfully
#
echo "info: start communicator daemon"
PATH="`pwd`/tests:${PATH}" \
ADVGETOPT_OPTIONS_FILES_DIRECTORY="../../BUILD/Debug/dist/share/communicator/options" \
../../BUILD/Debug/contrib/communicator/daemon/communicatord \
	--log-file "${COMMUNICATORD_LOG_FILE}" \
	--trace \
	--debug-all-messages \
	--my-address 127.0.0.1 \
	--services "../../BUILD/Debug/dist/share/communicator/services" \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--communicator-plugin-paths "`pwd`/../../BUILD/Debug/dist/lib/communicator/plugins" \
	--logger-plugin-paths "`pwd`/../../BUILD/Debug/dist/lib/snaplogger/plugins" \
	--timedate-wait-command "../../BUILD/Debug/dist/bin/timedate-wait" \
	--unix-listen "${COMMUNICATORD_SOCK}" &

# Start the fluid-settings daemon
#
echo "info: start fluid-settings"
../../BUILD/Debug/contrib/fluid-settings/daemon/fluid-settings \
	--log-file "${FLUID_SETTINGS_LOG_FILE}" \
	--trace \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--definitions "../../BUILD/Debug/dist/share/fluid-settings/definitions" \
	--communicator-listen "cd://`pwd`/${COMMUNICATORD_SOCK}" &

# Start the proxy daemon
#
echo "info: start prinbee proxy"
../../BUILD/Debug/contrib/prinbee/proxy/prinbee-proxy \
	--log-file "${PROXY_LOG_FILE}" \
	--trace \
	--owner alexis:alexis \
	--prinbee-path "`pwd`/tmp" \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--cluster-name "test_cluster" \
	--communicator-listen "cd://`pwd`/${COMMUNICATORD_SOCK}" &

# Start the actual daemon
#
echo "info: start prinbee daemon"
../../BUILD/Debug/contrib/prinbee/daemon/prinbee-daemon \
	--log-file "${DAEMON_LOG_FILE}" \
	--trace \
	--owner alexis:alexis \
	--prinbee-path "`pwd`/tmp" \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--cluster-name "test_cluster" \
	--communicator-listen "cd://`pwd`/${COMMUNICATORD_SOCK}" &

# Now run the pbql command
#
echo "info: start pbql..."
../../BUILD/Debug/contrib/prinbee/cui/pbql \
	--log-file "${CUI_LOG_FILE}" \
	--trace \
	--documentation "../../BUILD/Debug/dist/share/doc/prinbee/cui/" \
	--path-to-message-definitions "../../BUILD/Debug/dist/share/eventdispatcher/messages" \
	--communicator-listen "cd://`pwd`/${COMMUNICATORD_SOCK}"

# Stop everything except the communicator daemon
#
echo "info: ed-signal to broadcast STOP to all clients..."
../../BUILD/Debug/contrib/eventdispatcher/tools/ed-signal \
	--console \
	--trace \
	--except-stack-collect complete \
	./STOP

# Ask the communicator daemon to shutdown
#
sleep 1
../../BUILD/Debug/contrib/eventdispatcher/tools/ed-signal \
	--console \
	--trace \
	communicatord/STOP

sleep 1
if pgrep -u "${USER}" communicatord >/dev/null
then
	echo "info: ed-stop to send Ctrl-C to the communicatord..."
	../../BUILD/Debug/contrib/eventdispatcher/tools/ed-stop \
		--process-name \
		--service communicatord
fi

# Make sure it all ends
#
echo "info: waiting for background services to exit"
wait

# If necessary, reset the TTY, just in case
#
#sane-tty
