#!/bin/bash
echo "Input file contents:"
cat $1


# Open the files with file descriptors 3, 4, and 5
exec 3< $2
exec 4< $3
exec 5< $4

# Loop until all files are exhausted
echo "Output files concated contents:"

while true; do
  # Read a line from each file
  read -u 3 line1
  read -u 4 line2
  read -u 5 line3
  
  # Break the loop if all files are exhausted
  if [ -z "$line1" ] && [ -z "$line2" ] && [ -z "$line3" ]; then
    break
  fi
  
  # Print the lines
  [ -n "$line1" ] && echo "$line1"
  [ -n "$line2" ] && echo "$line2"
  [ -n "$line3" ] && echo "$line3"
done

# Close the file descriptors
exec 3<&-
exec 4<&-
exec 5<&-