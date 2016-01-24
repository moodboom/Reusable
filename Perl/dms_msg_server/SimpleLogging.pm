package SimpleLogging;

use strict;
$|++;

my $newline = "\cM\cJ";  # CRLF for Windoze

sub log_it
{
	my ($msg,$fn,$on) = @_;
	
	if($on)
	{
		my $now_string = localtime;
		open FILE , '>>'.$fn;
		print FILE $now_string.'| '.$msg.$newline;
		close FILE;
	}
}

1;