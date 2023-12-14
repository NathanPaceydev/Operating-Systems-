#!/bin/bash

# MD5 hash to match
target_hash="8afe37a36b4c052e197e98fd322ccb8f"

# Array of potential keywords derived from the provided information
keywords=("Samgault" "Gault" "April" "Grace" "Jack" "Astors" "Trip" "Sharon" "Arizona" "Ruby" "Volkswagen" "Jetta15" "March" "Jetta" "Ford" "Focus" "Sam" "Modesto" "Sgaulsoft" "Software" "SamGault" "Mmmbop" "April" "Cashier" "Tacos" "Pretzels" "Purple" "Spot" "Jackastor" "Winter" "Love" "Hanson")

# Specific numbers to append
# Birthday April 15, 1997
numbers=(209 11 0123 1234 1 2 3 0 8 6 86 68 772023 77 7 202377 123 97 1997 15 151997 1597 497 41597 4151997 1541997 15497 1997415 4 26 6 2009 152009 200915 2023 1541997 4151997)

# Function to check hash
check_hash() {
    local keyword=$1
    
    for num in "${numbers[@]}"; do
        local candidate="${keyword}${num}"
        local hash=$(echo -n "${candidate}" | md5sum | awk '{print $1}')
        echo $candidate
        echo -n "${candidate}" | md5sum | awk '{print $1}'
        if [ "$hash" == "$target_hash" ]; then
            echo "Password found: $candidate"
            return 0
        fi

    done
    return 1
}

# Iterate through keywords and check each
for keyword in "${keywords[@]}"; do
        hash=$(echo -n "${keyword}" | md5sum | awk '{print $1}')
        echo $keyword
        echo -n "${candidate}" | md5sum | awk '{print $1}'
        if [ "$hash" == "$target_hash" ]; then
            echo "Password found: $candidate"
            return 0
        fi
    if check_hash "$keyword"; then
        exit 0
    fi
done

echo "Password not found."
