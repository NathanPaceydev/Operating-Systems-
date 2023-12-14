#!/bin/bash

# Check if at least one argument is provided
if [ $# -lt 1 ]; then
  echo "Usage: $0 file1 [file2 ...]"
  exit 1
fi

# Initialize sum to 0
total_sum=0

# Loop through each file provided as an argument
for file in "$@"; do
  # Check if the file exists
  if [ ! -f "$file" ]; then
    echo "Error: File $file not found" >&2
    continue
  fi
  
  # Sum the contents of the file
  file_sum=$(awk '{sum += $1} END {print sum}' "$file")
  
  # Add the sum of the current file to the total sum
  total_sum=$((total_sum + file_sum))
  
  # Print the sum of the current file
  echo "Sum of $file: $file_sum"
done

# Print the total sum
echo "Total sum: $total_sum"
