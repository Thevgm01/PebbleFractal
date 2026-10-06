#!/bin/bash

if [ "$#" -lt 1 ]; then
    echo "Must pass in desired emulator."
    exit 1
fi

pebble build
if [ $? -ne 0 ]; then
  echo "Aborting install."
  exit 1
fi

pebble install --emulator $1
