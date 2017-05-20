#!/bin/bash

set -euo pipefail

trap 'echo "Decoder failed" >&2' ERR

cd "$(dirname "$0")"

declare -r out='kissb'

export decoder_binary="$(mktemp)"

if [ "${csv:-}" ]; then
	# Print CSV header
	source=y passtime=y ../decode
fi

for file in "$out"/*.kss; do
(
	declare -ra info=( x $(echo "$(basename "$file")" | perl -ne 'print if s{^(\w+)-(\d\d)(\d\d)(\d\d\d\d)_(\d\d)(\d\d)(\d\d)?\.kss$}{$1 $4-$3-$2T$5:$6:00Z}g') )
	echo -e "\e[1;36m${file#$out/}\e[0m" >&2
	source="${info[1]:-?}" passtime="${info[2]:-?}" ../decode "$file"
) || true
done

for file in opencosmos/*_beacon-raw_*.bin; do
(
	declare -ra info=( x $(echo "$(basename "$file")" | perl -ne 'print if s{^.*_beacon-raw_(\d+)\.\d+\.bin$}{$1}') )
	echo -e "\e[1;36m${file#opencosmos/}\e[0m" >&2
	source='opencosmos' passtime="${info[1]:-?}" raw=yes ../decode "$file"
) || true
done
