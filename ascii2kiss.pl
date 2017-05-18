#!/usr/bin/perl

use strict;
use warnings;

while (<>) {
	if (/^[\S]/) {
		print "C0";
		next;
	}
	next unless s{^\s*\d+\s*>\s*}{};
	s{\s}{}g;
	chomp;
	print
}
