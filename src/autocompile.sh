#!/bin/sh
###########################################################################
#
# .SYNOPSYS
#     Compile Arduino sketches found in the "Firmware" path
#     using FQBN configuration files.
#     To be run in a GitHub workflow.
#
# .AUTHORS
#     José Agustin (https://github.com/janc18)
#     Ángel Fernández Pineda. Madrid. Spain. 2024.
#
# .LICENSE
#     Licensed under the EUPL
#
# .DEPENDENCIES
#     Arduino-cli
#
# .DEPENDENCIES when running outside GitHub Actions
#     python
#     pip
############################################################################

# Configuration parameters
esp_core_version=3.1.1
nimble_version=2.2.0

############################################################################

# Constants for colored output
# See: https://www.shellhacks.com/bash-colors/
EOC="\033[0m" # End of color
COLOR1="\033[1;32m" # Green+bold
COLOR2="\033[33m" # Brown
COLOR3="\033[32m" # Green

# Create temporary file to store compiler output
TMP_FILE=$(mktemp -q /tmp/bar.XXXXXX)
if [ $? -ne 0 ]; then
    echo "❌ Can't create temporary file. Exiting."
    exit 1
fi

# Make sure we are in the correct filesystem path
cd "$(dirname "$(realpath "$0")")"

# Install Arduino dependencies
echo "${COLOR2}⛏ ${COLOR1}Installing Arduino dependencies${EOC}"
echo "${COLOR2}🛈 ${COLOR1}ESP-Arduino core version${EOC}: $esp_core_version"
echo "${COLOR2}🛈 ${COLOR1}NimBLE-Arduino version${EOC}: $nimble_version"

arduino-cli config init --additional-urls https://downloads.arduino.cc/packages/package_index.json> /dev/null
arduino-cli core update-index
arduino-cli core install esp32:esp32@$esp_core_version
arduino-cli lib install NimBLE-Arduino@$nimble_version

# Create symbolic links
echo "${COLOR2}⛏ ${COLOR1}Creating symbolic links to source files${EOC}"
./MakeSymLinks.sh > /dev/null 2>&1

# Workaround for testing outside GitHub Actions
if [ -z ${GITHUB_ACTIONS} ]; then
    echo "${COLOR2}🛈 ${COLOR1}Running outside the GitHub workflow context. Enabling Python workaround.${EOC}"
    python -m venv . > /dev/null 2>&1
    source ./bin/activate
    pip install pyserial > /dev/null 2>&1
fi

# Compilation loop
echo "🔎 ${COLOR1}Looking for Arduino sketches${EOC}"
ERROR_COUNT=0
for sketch in $(find ./Firmware/ -type f -name "*.ino"); do
    filename=$(basename "$sketch")  # Extract only the .ino filename
    sketch_dir=$(dirname "$sketch")  # Get the directory of the .ino file
    config_file="$sketch_dir/.fqbn"  # Path to the .fqbn file in the same directory

    # Check if the ".fqbn" file exists
    if [ -f "$config_file" ]; then
        config_content=$(head -n 1 "$config_file")
    else
        echo "${COLOR2}🛈 ${COLOR3}Ignoring sketch${EOC} $filename"
        continue
    fi
    echo "${COLOR2}⛏ ${COLOR3}Compiling${EOC} $filename ${COLOR2}🛈 ${COLOR3}FQBN:${EOC} $config_content"
    # Compile the .ino file
    if arduino-cli compile -b $config_content "$sketch" >$TMP_FILE 2>&1; then
        echo "✅ Success"
    else
        ERROR_COUNT=$((ERROR_COUNT+1))
        echo "❌ Error. Compiler output follows:"
        cat $TMP_FILE | sed  's|(/[^ ]+/)+||g'
    fi
    config_content=""
done

# Finalization
if [ -z ${GITHUB_ACTIONS} ]; then
    echo "${COLOR2}🛈 ${COLOR1}Running outside the GitHub workflow context. Deactivating Python.${EOC}"
    deactivate
fi

## remove temporary file
rm -f $TMP_FILE >/dev/null 2>&1

exit $ERROR_COUNT