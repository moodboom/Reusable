#!/usr/bin/perl -w
%hsh = ("foo"=>"bar");

map{ print } keys %hsh;

## get a reference to the value of the key "foo"
$test = \$hsh{foo};

## deref the ref and assign something new to it
${$test} = "new";

## print out the values - did it work?  SURE DID!
map{ print } values %hsh;

