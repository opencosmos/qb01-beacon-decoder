#!/bin/bash

set -e

CSV=y ./data/convert.sh 2>/dev/null 1>beacons.csv
