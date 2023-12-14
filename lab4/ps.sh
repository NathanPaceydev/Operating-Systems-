#!/bin/bash
#ELEC377 - Operating Systems
#Lab - Shell Scripting, ps.sh

# Program desicription: 
# This script is designed to display a list of all running processes on a Linux system.
# It provides options to include additional information about each process, such as
# resident set size, command name, command line, and group ID.


# Phase 1. Parse command line arguments to set flags

# initalize variables to 'no'
ShowRSS="no"
ShowExecComm="no"
ShowStartComm="no"
ShowGroupID="no"

# loop through shell command input using shift
while(($# > 0)); do 
    if [[ $1 == "-rss" ]]; then
        ShowRSS="yes"
    elif [[ $1 == "-comm" ]]; then 
        ShowExecComm="yes"
    elif [[ $1 == "-command" ]]; then 
        ShowStartComm="yes"
    elif [[ $1 == "-group" ]]; then 
        ShowGroupID="yes"
    else 
        echo "Error: $1 is not recognized!"
        exit 1
    fi 
    shift 
done

# Check for mutually exclusive flags
if [[ $ShowExecComm == "yes" && $ShowStartComm == "yes" ]]; then
    echo "Error: -comm and -command flags are mutually exclusive."
    exit 1
fi

# Debug: Echo the set flags
echo "ShowRSS: $ShowRSS"
echo "ShowExecComm: $ShowExecComm"
echo "ShowStartComm: $ShowStartComm"
echo "ShowGroupID: $ShowGroupID"

# Temporary file for storing output
tempFile="/tmp/tmpPs$$.txt"
rm -f $tempFile

# Phase 2. Iterate over process directories in /proc
# create a for loop to iterate over the process dirs
# information about the process used are in the files status and cmdline

for p in /proc/[0-9]*; do
    # check that the directory still exists
    if [[ -d $p ]]; then
        # Debug: Echo the process directory
        # commented out to compare uncomment to see directories
        # echo "Process dir is $p"

        # Phase 3. Use grep and sed to display process info
        pid=$(grep '^Pid:' $p/status | sed 's/[^0-9]*//g')
        user_id=$(grep '^Uid:' $p/status | awk '{print $2}')
        group_id=$(grep '^Gid:' $p/status | awk '{print $2}')
        rss=$(grep '^VmRSS:' $p/status | awk '{print $2}')
        comm=$(grep '^Name:' $p/status | sed 's/Name:\s*//')

        # Handle cmdline, replacing null characters with spaces
        cmdline=$(tr '\0' ' ' < $p/cmdline)
        # check if the cmd line is empty
        if [[ -z $cmdline ]]; then
            cmdline=$(grep '^Name:' $p/status | sed 's/Name:\s*//')
        fi

        # Set rss to 0 if it is empty
        if [[ -z $rss ]]; then
            rss=0
        fi

        # Phase 3b: Convert numeric user and group IDs to symbolic names
        user=$(grep ":x:$user_id:" /etc/passwd | cut -d: -f1)
        group=$(grep ":x:$group_id:" /etc/group | cut -d: -f1)

        # probably more dependable method across different OS to use getent
        # but not what the lab specified for 3b.
        #group=$(getent group | awk -F: '$3 == '"$group_id"' {print $1}')
        #user=$(getent passwd | awk -F: '$3 == '"$user_id"' {print $1}')


        # Phase 4. Output 
        # print all the variables to Temperary File
        printf "%-6s %-10s %-6s %-8s %-20s %-50s\n" $pid $user $group $rss $comm "$cmdline" >> $tempFile 

    fi
done

# Phase 5. Print header based on inputs and display sorted results
printf "%-6s %-10s " "PID" "USER"
[[ $ShowGroupID == "yes" ]] && printf "%-6s " "GROUP"
[[ $ShowRSS == "yes" ]] && printf "%-8s " "RSS"
[[ $ShowExecComm == "yes" ]] && printf "%-20s " "COMM"
[[ $ShowStartComm == "yes" ]] && printf "%-50s " "COMMAND"
echo ""

# sort and display results using sort -n and awk
# sort and display results using sort -n and awk
sort -k1,1n $tempFile | awk -v ShowGroupID="$ShowGroupID" -v ShowRSS="$ShowRSS" -v ShowExecComm="$ShowExecComm" -v ShowStartComm="$ShowStartComm" '{
    printf "%-6s %-10s ", $1, $2;
    if (ShowGroupID == "yes") printf "%-6s ", $3;
    if (ShowRSS == "yes") printf "%-8s ", $4;
    if (ShowExecComm == "yes") printf "%-20s ", $5;
    if (ShowStartComm == "yes") printf "%-50s ", $6;
    print "";
}'

# Clean up by removing the temp file
rm -f $tempFile
 