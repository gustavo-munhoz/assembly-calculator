#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <EXPRESSIONS_FILE.txt>"
    exit 1
fi

INPUT_FILE="$1"

./build/proj1 "$INPUT_FILE"
if [ $? -ne 0 ]; then
    echo "Error executing proj1"
    exit 1
fi

ASM_FILE="${INPUT_FILE%.*}.asm"
HEX_FILE="${INPUT_FILE%.*}.hex"

echo "Assembly generated."
echo "Assembling using avra to generate HEX: $HEX_FILE"

avra -o "$HEX_FILE" "$ASM_FILE"
if [ $? -ne 0 ]; then
    echo "Error assembling with avra."
    exit 1
fi

PORT="/dev/cu.usbserial-A5069RR4"
BAUD="57600"

echo "Uploading HEX to Arduino at port $PORT with baud rate $BAUD"
avrdude -v -patmega328p -carduino -P "$PORT" -b "$BAUD" -D -U flash:w:"$HEX_FILE":i

if [ $? -ne 0 ]; then
    echo "Error uploading HEX to Arduino."
    exit 1
fi

echo "Uploaded successfully!"
echo "Cleaning build files..."

rm "${INPUT_FILE%.*}.obj" "${INPUT_FILE%.*}.eep.hex" "$HEX_FILE" 

echo "Done!"
