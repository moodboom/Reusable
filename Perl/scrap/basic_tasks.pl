#!/usr/bin/perl -w
use strict;
$|++;

#print "Content-type: text/html\n\n";
print "<HTML>\n";



# How do we parse a command line argument?
#
# MDM Here's how to do it for the following command-line formats:
#  1) URL's, in the form: http://www.thedigitalmachine.com/cgi-bin/perl/basic_tasks.pl?param1=wahoooo&var2=val2
#  2) Command line calling, in the form: ./basic_tasks.pl param1=wahooo
# CGI parameters taken from the URL:
use CGI qw(:standard);
my $param1 = param('param1');
print $param1."\n";






# How do we call a subroutine?
test("one");
test(1);

sub test( my $parameter1 )
{
	print "Inside test... param = ".$parameter1."\n";
}





## How do we derive an object?



