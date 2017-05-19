#!/bin/bash

set -euo pipefail

cd "$(dirname "$0")"

declare -r out='kissb'

mkdir -p "$out"

function convert {
	local -r group="$1"
	local -r ext="$2"
	shift 2
	local -ra cmd=( "$@" )
	echo " * $group" >&2
	for file in "$group"/*."$ext"; do
		echo "    - $file" >&2
		"${cmd[@]}" "$file" > "$out/$group-$(basename "${file%.$ext}").kss"
	done
}

function run_decoder {
	bash ../decode.c "$@"
}

convert aalto txt ./ax25a2kissb.pl
convert tartu dmp ./kissa2kissb.pl
convert tartu kss cat
convert japan log ./unax25a2kissb.pl

set +e

if [ "${CSV:-}" ]; then
	# Print CSV header
	source=y passtime=y run_decoder
fi

for file in "$out"/*.kss; do
(
	declare -ra info=( x $(echo "$(basename "$file")" | perl -ne 'print if s{^(\w+)-(\d\d)(\d\d)(\d\d\d\d)_(\d\d)(\d\d)(\d\d)?\.kss$}{$1 $4-$3-$2T$5:$6:00Z}g') )
	echo -e "\e[1;36m${file#$out/}\e[0m" >&2
	source="${info[1]:-?}" passtime="${info[2]:-?}" run_decoder "$file"
) || true
done
