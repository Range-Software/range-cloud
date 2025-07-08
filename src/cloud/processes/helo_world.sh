#!/bin/bash

readonly MY_PATH="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"

. "$MY_PATH/lib.sh"

echo "Hello world."
echo "Command: $0 $@"
