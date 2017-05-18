#!/usr/bin/perl

use strict;
use warnings;

### Convert ASCII raw KISS to binary raw KISS

while (<>) {
	if (/^[\S]/) {
		print chr(0xC0);
		next;
	}
	next unless s{^\s*\d+\s*>\s*}{};
	s{\s+}{}g;
	chomp;
	s{[0-9a-fA-F]{2,2}}{chr(hex($&))}ge;
	print
}
