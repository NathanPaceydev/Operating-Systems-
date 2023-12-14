#!/bin/bash

# Check if at least one argument is provided
if [ $# -lt 1 ]; then
  echo "Usage: $0 file1 [file2 ...]"
  exit 1
fi

# Concatenate the contents of all files, sort the result, and print
cat "$@" 2>/dev/null | sort -n
