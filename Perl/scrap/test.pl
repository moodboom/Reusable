#!/usr/bin/perl
use strict;

print "test";

my $test = " / blah blah / blah blah ";
$test =~ s/ (.*?\/)/_\1/g;
print $test;

$test = "/1/2/3";
my @testparts = split /\//, $test;
print @testparts;
print $testparts[2];

$test = $test.$test;
print $test;
