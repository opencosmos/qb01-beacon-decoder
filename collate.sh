#!/bin/bash

set -euo pipefail

trap 'echo "Collate failed" >&2' ERR

make

cd "$(dirname "$0")"

./data/convert.sh
csv=y ./data/decode.sh >beacons.csv
