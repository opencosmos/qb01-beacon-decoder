# qb01 beacon decoder

Uses RS decoder from Phil Karn's "feclib".

## Usage

To compile:

	make

To run:

	[environment] ./decode infile.kss

To prevent warnings/errors from messing up the output, redirect stderr:

	csv=y ./decode example.kss 2>/dev/null

It is currently designed to read binary KISS files (which include AX.25 framing) and extract beacons from them, and raw files in the structure used by the satellite.
It may be useful to alter it to read ASCII KISS and other formats also.

Data in other formats may possibly be converted to binary KISS AX.25 via some of the perl scripts in `data/`.

The script `./collate.sh` converts all data in the `data/` folder, decodes it, and produces a CSV file called `beacons.csv`.

## Callsign issue

The qb01 beacon callsign fields are not bit-shifted, so unfortunately an AX.25-compliant TNC will have trouble decoding them.
Instead, simply discard the AX.25 framing (first 16 bytes + last 2 bytes).
The next level is Reed-Solomon FEC so the AX.25 CRC16 is not essential.

If your TNC insists on deframing the AX.25 itself, it will probably read the CSP header and the time field as a "via" callsign, so alter the decoder appropriately to handle the lack of the "time" field.

Specify environment variable "csv" in order to export beacons as machine-readable CSV format instead of the human-readable default format.
	Be sure to redirect stderr away if using CSV output, so warnings/errors don't pollute your CSV file.

When supplying beacon data to Open Cosmos, please indicate the time (and ideally, also location) of the reception!

You can supply the data via the email address on the details page [http://open-cosmos.com/se01/](http://open-cosmos.com/se01/) or via pull-requests to this repository (please use the data/ folder and if possible, binary KISS format).

If you send a pull-request, please create a new folder for your team/organisation/yourself as appropriate and put your data in there.
Also, if you can provide a script to translate your data to binary KISS encapsulating AX.25 frames (or use one of the existing perl scripts) that would be great but it is not essential.

## Note

The `find_beacon_len` function introduces a possible buffer overrun bug, with the potential to overrun by up to 32 bytes.  This is solved by requiring the input data to be at least 32 bytes smaller than the buffer which it is read into.  The buffer is relatively huge so this should not create problems.

 - _Mark Cowan @ Open Cosmos_

# qb01 beacon description

See `QB50 SE01 beacon description - Open Cosmos.pdf` for detailed information.

| Parameter                  | Value                                                               |
|----------------------------|---------------------------------------------------------------------|
| Modulation                 | GFSK                                                                |
| Modulation index           | 0.6667                                                              |
| Packet format              | AX-25*, CSP                                                         |
| Encoding                   | NRZI with stuffing / G3RUH scrambled for AX.25, RS(223,255) for CSP |
| Carrier-Frequency          | 435.800 MHz                                                         |
| Nominal data rate          | 9600 baud                                                           |
| AX-25 Source Address Field | ON01SE                                                              |
| Interval                   | 10 s (LEOP), 30 s (during routine Operations [to be commanded])     |

# TLE

* Preliminary NSSDC ID: `1998-067LM`
* Preliminary NORAD catalog number: `42704`
