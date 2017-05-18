#!/usr/bin/perl

use strict;
use warnings;

### Convert ASCII AX.25 to binary raw KISS, assuming one packet per line

while (<>) {
	next unless m{^[0-9a-fA-F]{2,2}[0-9a-fA-F\s]+$};
	s{\s+}{}g;
	chomp;
	$_ = uc($_);
	s{[0-9a-fA-F]{2,2}}{$& }g;
	s{DB }{DB DD }g;
	s{C0 }{DB DC }g;
	s{\s+}{}g;
	$_ = "C0${_}C0";
	s{[0-9a-fA-F]{2,2}}{chr(hex($&))}ge;
	print;
}
