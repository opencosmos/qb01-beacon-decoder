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

convert aalto txt ./ax25a2kissb.pl
convert tartu dmp ./kissa2kissb.pl
convert tartu kss cat
convert tartu mkb cat
convert japan log ./unax25a2kissb.pl
