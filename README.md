# qb01 beacon decoder

Uses RS decoder from Phil Karn's "feclib".

The program is a self-compiling polyglot, so to use it just:

	bash beacon.c example.kss

To prevent warnings/errors from messing up the output, redirect stderr:

	bash beacon.c example.kss 2>/dev/null

It is currently designed to read binary KISS files (which include AX.25 framing) and extract beacons from them.
It may be useful to alter it to read ASCII KISS and other formats also.

The qb01 beacon callsign fields are not bit-shifted, so unfortunately an AX.25-compliant TNC will have trouble decoding them.
Instead, simply discard the AX.25 framing (first 16 bytes + last 2 bytes).
The next level is Reed-Solomon FEC so the AX.25 CRC16 is not essential.

If your TNC insists on deframing the AX.25 itself, it will probably read the CSP header and the time field as a "via" callsign, so alter the decoder appropriately to handle the lack of the "time" field.

Specify environment variable "CSV" in order to export beacons as machine-readable CSV format instead of the human-readable default format.
	Be sure to redirect stderr away if using CSV output, so warnings/errors don't pollute your CSV file.

When supplying beacon data to Open Cosmos, please indicate the time (and ideally, also location) of the reception!

You can supply the data via the email address on [http://open-cosmos.com/se01/](http://open-cosmos.com/se01/) or via pull-requests to this repository (please use the data/ folder and if possible, binary KISS format).

 - _Mark Cowan @ Open Cosmos_
