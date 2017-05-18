#!/usr/bin/perl

use strict;
use warnings;

### Convert ASCII deframed AX.25 to binary raw KISS, assuming that the first 8 bytes of payload have been interpreted as a VIA callsign.
### Also trims off weird 01 byte from start of each packet.

# Dummy framing
my $callsign = "4F 4E 30 31 53 45 00";
my $ax25_hdr = "$callsign $callsign 03 00";
my $ax25_crc = "00 00";
my $csp_hdr = "02 A2 DBDC 00";
my $time = "00 00 00 00"; # Let the Reed-Solomon recover the time

my $prefix = "$ax25_hdr $csp_hdr $time ";
my $suffix = "$ax25_crc ";

my $in = 0;

my @packets = ();
my $this = "";

while (<>) {
	if (not s{^\s*\d+\s*>\s*}{}) {
		push @packets, $this if $in;
		$in = 0;
		next;
	}
	$this = '' if not $in;
	$in = 1;
	s{\s+}{}g;
	chomp;
	$_ = uc($_);
	s{[0-9a-fA-F]{2,2}}{$& }g;
	s{DB }{DB DD }g;
	s{C0 }{DB DC }g;
	$this .= $_;
}

my $out = 'C0 ' . (join 'C0 ', (map { s{^01 }{}; "$prefix $_ $suffix"; } @packets)) . 'C0 ';
$out =~ s{\s+}{}g;
$out =~ s{[0-9a-fA-F]{2,2}}{chr(hex($&))}ge;
print $out;
