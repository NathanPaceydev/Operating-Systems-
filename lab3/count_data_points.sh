#!/bin/bash

# Check if at least one argument is provided
if [ $# -lt 1 ]; then
  echo "Usage: $0 file1 [file2 ...]"
  exit 1
fi

# Initialize total line count to 0
total_lines=0

# Loop through each file provided as an argument
for file in "$@"; do
  # Check if the file exists
  if [ ! -f "$file" ]; then
    echo "Error: File $file not found" >&2
    continue
  fi
  
  # Count the number of lines in the file
  num_lines=$(wc -l < "$file")
  
  # Add the number of lines in the current file to the total line count
  total_lines=$((total_lines + num_lines))
  
  # Print the number of lines in the current file
  echo "Number of lines in $file: $num_lines"
done

# Print the total number of lines
echo "Total number of lines: $total_lines"
