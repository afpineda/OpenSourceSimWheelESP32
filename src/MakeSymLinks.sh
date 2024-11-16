#!/bin/bash
THIS_PATH="$(dirname "$(realpath "$0")")"
INCLUDE_PATH="$THIS_PATH/include"
COMMON_PATH="$THIS_PATH/common"
INCLUDES_FILE="includes.txt"

get_required_links() {
    local path="$1"
    local links=()
    file_content=$(<"$path/$INCLUDES_FILE")
    if [ -n "$file_content" ] && [ "${file_content: -1}" != $'\n' ]; then
        file_content="$file_content"$'\n'
    fi
    while IFS= read -r line; do
        line=$(echo "$line" | sed 's/\r//;s/^[[:space:]]*//;s/[[:space:]]*$//')
        if [[ -n "$line" ]]; then
            if [[ "$line" == *.h ]]; then
                links+=("$INCLUDE_PATH/$line")
            else
                links+=("$COMMON_PATH/$line")
            fi
        fi
    done <<< "$file_content"
    echo "${links[@]}"
}

get_sketch_folders() {
    find "$THIS_PATH" -type f -name "*.ino" -exec dirname {} \; | sort | uniq
}


echo "Path: $THIS_PATH"

SKETCH_FOLDERS=$(get_sketch_folders)

for folder in $SKETCH_FOLDERS; do
    INCLUDES_FILE_PATH="$folder/$INCLUDES_FILE"
    if [[ -f "$INCLUDES_FILE_PATH" ]]; then
        echo "Processing folder: $folder"

        req_links=$(get_required_links "$folder")

        find "$folder" -type f \( -name "*.cpp" -o -name "*.h" \) -exec rm -f {} \;

        for link_spec in $req_links; do
            target="$folder/$(basename "$link_spec")"
            
            if ln -s "$link_spec" "$target" 2>/dev/null; then
                echo "Created symlink: $link_spec -> $target"
            else
                if [ ! -w "$target" ]; then
                    cp -f "$link_spec" "$target"
                    echo "Copied: $link_spec -> $target"
                else
                    echo "Error: $link_spec not found"
                fi
            fi
        done
    else
        echo "No includes.txt found in folder: $folder"
    fi
done
