use strict;

use File::Find;	# for find(), which does all our dir recursion

# Set up the default to use the current directory
# but only if one wasn't provided on the command line.
@ARGV = ('.') unless @ARGV;

my $linecount;
my @filelist;

$file = "test.txt";

open(FILE, "< $file") or die "Can't open $file: $!";

# We don't even need a count var, we can use the "special variable" [$.]
# $count++ while <FILE>;
1 while <FILE>;

print "line count for $file is $.";