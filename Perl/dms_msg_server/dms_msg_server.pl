#!perl -w
use strict;
$|++;

# MDM Simple web service to allow messages to be sent to OMS operators.
# This uses HTTP::Server::Simple to rig up a very basic web server.
# It is intentionally single-threaded and will not handle a large volume of rapid requests.
#
#
# WEB SERVICE PROVIDER STARTUP
# ----------------------------
# The web service should be started on the machine that also runs the NM-DMS server apps.
# The following environment variables must be set:
#
#		CADOPS_HOME				Used to find the [generalSend] executable, which is called
#								to actually display a message to the NM-DMS operator.
#								[/bin/generalSend] is appended.  
#								Eg: /pkg_TSC2_opt/cadops1
#
#		CADOPS_LOCAL_DATA_DIR	Base directory for log file.  
#								All msgs from DMS to OMS are logged.
#								[/logs/dms_msg_server.log] is appended.  
#								Eg: /pkg_TSC2_opt/var1
#
#		DMS_MSG_SVR_PORT		Server port.  The server runs on the host where the
#								server script is started, using this port.
#								Must be one of: 7540 (production), 7542 (development)
#
# The server validates the environment variables, echoing them to the console and logfile
# on startup.  If any are not set, the server prints an error and does not start.
#
#
# WEB SERVICE CONSUMER CALLS
# --------------------------
# Only one specific URL path is understood: http://{host}:{port}/send_message_alert
# The only security comes from a key provided by the client that must match server.
# Parameters must be provided via POST, and are normalized and validated as follows:
#
#	user		text string from 1 to 30 characters
#	text		text string from 1 to 500 characters
#	severity	text string, must be one of: Information | Warning | Error
#	key			text string, must be: 23409851-056791264
#
# Any invalid or missing parameters result in a blank response from the server.
#
#
# FURTHER NOTES
# -------------
#
# 1) OMS logout/login does not fix the operator subscription used by generalSend. 
# Operators must close and reopen Ormap for operator-specific messages to work.
# Currently, logout/login is not used at PGN, as it causes other problems as well.
#
# 2) The maximum message length allowed by generalSend appears to be approx 154 chars.
#
# 3) Any double quotes are converted to single quotes before calling generalSend.
#

use SimpleLogging;
use MessageAlertWebServer;

# Grab our required environment variables.
use Env qw(CADOPS_HOME CADOPS_LOCAL_DATA_DIR DMS_MSG_SVR_PORT);

my $log_on = 1;
my $log_fn = $CADOPS_LOCAL_DATA_DIR.'/logs/dms_msg_server.log';

my $newline = "\cM\cJ";  # CRLF for Windoze

# Validate environment variables before we start up.
#use Env qw(CADOPS_HOME CADOPS_LOCAL_DATA_DIR DMS_MSG_SVR_PORT);
if (!$CADOPS_HOME || !$CADOPS_LOCAL_DATA_DIR || !$DMS_MSG_SVR_PORT)
{
	print $newline."*******".$newline;
	print "*ERROR: Environment variables were not set, please refer to documentation.".$newline;
	print "*******".$newline.$newline;
	exit 1;
}
if (
		not (-d $CADOPS_HOME)
	||	not (-d $CADOPS_LOCAL_DATA_DIR)
	||  not ($DMS_MSG_SVR_PORT == 7540 || $DMS_MSG_SVR_PORT == 7542)
){
	print $newline."*******".$newline;
	print "*ERROR: Invalid environment variable values, please refer to documentation.".$newline;
	print "*******".$newline.$newline;
	exit 2;
}

# start the server
my $pid = MessageAlertWebServer->new($DMS_MSG_SVR_PORT)->background();

# Finish initial output.
my $host = `hostname`;
chomp($host);
SimpleLogging::log_it("----------------------------"						,$log_fn,$log_on);
SimpleLogging::log_it("** dms_msg_server STARTED **"						,$log_fn,$log_on);
SimpleLogging::log_it("----------------------------"						,$log_fn,$log_on);
SimpleLogging::log_it("http://$host:$DMS_MSG_SVR_PORT/send_message_alert"	,$log_fn,$log_on);
SimpleLogging::log_it("logfile: $log_fn"									,$log_fn,$log_on);
SimpleLogging::log_it("CADOPS_HOME: $CADOPS_HOME"							,$log_fn,$log_on);
SimpleLogging::log_it("pid: $pid"											,$log_fn,$log_on);
SimpleLogging::log_it("----------------------------"						,$log_fn,$log_on);
print "Server started, see logfile for further details:$newline$newline   $log_fn$newline$newline";
