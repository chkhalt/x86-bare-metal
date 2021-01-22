#!/bin/bash

INPUT=$1
strip $INPUT

# get .text offset
OFFSET=$(objdump -x $1 | grep .text | head -n 1 | awk '{ print $6 }')
OFFSET=$((16#$OFFSET))

# if necessary remove part of the header
if [ $(($OFFSET & 0xfffff000)) != 0 ]; then
	dd if=$INPUT of=$INPUT.tmp bs=1 skip=$(($OFFSET & 0xfffff000))>/dev/null 2>&1
	mv $INPUT.tmp $INPUT
	OFFSET=$(($OFFSET & 0x00000fff))
fi

if [ $OFFSET == 0 ]; then
	exit 0
fi

# place jump at the beginning
cat << EOF | python -
import struct
from binascii import unhexlify

with open('$INPUT', 'r+b') as f:
	f.write(unhexlify('e9'))
	f.write(struct.pack('<L', $OFFSET-5))
EOF
