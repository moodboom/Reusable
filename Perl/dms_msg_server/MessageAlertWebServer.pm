package MessageAlertWebServer;

use strict;
$|++;

use SimpleLogging;

use HTTP::Server::Simple::CGI;
use base qw(HTTP::Server::Simple::CGI);

# Grab our required environment variables.
use Env qw(CADOPS_HOME CADOPS_LOCAL_DATA_DIR DMS_MSG_SVR_PORT);

my $log_on = 1;
my $log_fn = $CADOPS_LOCAL_DATA_DIR.'/logs/dms_msg_server.log';

# MDM Map url path to subroutines.
# We only have one path and it must be correctly specified.
my %dispatch = (
	 '/send_message_alert' => \&send_message_alert,
	 # ...
);
 
sub handle_request 
{
	 my $self = shift;
	 my $cgi  = shift;
   
	 my $path = $cgi->path_info();
	 my $handler = $dispatch{$path};
 
	 if (ref($handler) eq "CODE") {
		 print "HTTP/1.0 200 OK\r\n";
		 $handler->($cgi);
         
	 } else {
		 print "HTTP/1.0 404 Not found\r\n";
		 print $cgi->header,
			   $cgi->start_html('Not found'),
			   $cgi->h1('Not found'),
			   $cgi->end_html;
	 }
}
 
sub send_message_alert 
{
	my $cgi  = shift;   # CGI.pm object
	return if !ref $cgi;

	my $user = $cgi->param('user');
	my $text = $cgi->param('text');
	my $severity = $cgi->param('severity');
	my $key = $cgi->param('key');

	
	# ----------------------
	# NORMALIZE AND VALIDATE
	# ----------------------
	if (
			(!$key || !$user || !$text || !$severity)
		||	($key ne '23409851-056791264')
		||	not ($severity eq 'Information' || $severity eq 'Error' || $severity eq 'Warning') 
	){
		$cgi->header,
		$cgi->start_html("Invalid"),
		$cgi->end_html;
		return;
	}

	# Cleanse the text.
	# We replace double quotes, and we don't allow funky chars, so that
	# we don't have any issues when we call generalSend.
	$text =~ s/"/'/g;
	# The first strips heavily, the second only lightly.
	# $text =~ s/[^A-Za-z0-9\-\.]//g;
	$text =~ 's/[\x00-\x08\x0b-\x0c\x0e-\x1f\x7f]//g';

	# Cleanse the operator.
	# It can be global [*] or alphanumeric and basic punctuation.
	$user =~ s/[^\*A-Za-z0-9\-\.]//g;

	# We don't want buffer overruns or any other hackery.
	$user = substr $user, 0, 30;	# limit to  30 chars
	$text = substr $text, 0, 500;	# limit to 500 chars
	# ----------------------


	# Log the request.
	SimpleLogging::log_it("$severity sent to $user: $text",$log_fn,$log_on);

	# DEBUG Provide parameter echo in html.
	print 
		$cgi->header,
		$cgi->start_html("Message Alert"),
		$cgi->h1("$severity sent to $user: $text"),
		$cgi->end_html;
		
	# Now call generalSend.
	my $my_cmd = "cd $CADOPS_HOME/bin/ && ./generalSend /operator/$user/locate 0 \"$text\"";
	`$my_cmd`;
}

1;