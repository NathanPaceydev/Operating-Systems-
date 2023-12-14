#!/bin/bash

# Check if two arguments are passed
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <file1> <file2>"
    exit 1
fi

# Assign the arguments to variables
file1="$1"
file2="$2"

# Function to clean up the file
clean_file() {
    sed 's/^[[:space:]]*//;s/[[:space:]]*$//' "$1" | sort -u
}

# Read the cleaned up file1 and file2 into arrays
mapfile -t lines1 < <(clean_file "$file1")
mapfile -t lines2 < <(clean_file "$file2")

# Helper function to search for a PID in an array of lines
search_pid() {
    local pid="$1"
    shift
    local lines=("$@")

    for line in "${lines[@]}"; do
        if [[ "$line" =~ ^$pid[[:space:]] ]]; then
            echo "$line"
            return 0
        fi
    done

    return 1
}

# Compare PIDs and usernames
echo "Comparing PIDs and usernames:"
for line1 in "${lines1[@]}"; do
    pid1=$(echo "$line1" | awk '{print $1}')
    user1=$(echo "$line1" | awk '{print $2}')
    
    line2=$(search_pid "$pid1" "${lines2[@]}")
    if [[ -n "$line2" ]]; then
        user2=$(echo "$line2" | awk '{print $2}')
        if [[ "$user1" != "$user2" ]]; then
            echo "PID $pid1 has different usernames: $user1 vs $user2"
        fi
    else
        echo "PID $pid1 with username $user1 is unique to $file1"
    fi
done

# Find PIDs unique to file2
echo "PIDs unique to $file2:"
for line2 in "${lines2[@]}"; do
    pid2=$(echo "$line2" | awk '{print $1}')
    
    if ! search_pid "$pid2" "${lines1[@]}" >/dev/null; then
        echo "$line2"
    fi
done
