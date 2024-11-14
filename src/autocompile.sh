#!/bin/sh
# Required dependencies:
# - pip
# - python
# - arduino-cli

#Config parameters
esp_core_version=3.0.1
nimble_version=1.4.2

#Displaying config parameters
echo "🗂️ Compiling with the ESP32 core $esp_core_version version"
echo "🗂️ Compiling with NimBLE-Arduino $nimble_version version"
#Installing dependencies
arduino-cli config init --additional-urls https://downloads.arduino.cc/packages/package_index.json> /dev/null 
arduino-cli core update-index 
arduino-cli core install esp32:esp32@$esp_core_version 
arduino-cli lib install NimBLE-Arduino@$nimble_version
#Preparation to compile (Making Symsliks, activating python virtual enviroment , installing pyserial)
./MakeSymLinks.sh > /dev/null 2>&1
python -m venv . > /dev/null 2>&1
source ./bin/activate 
pip install pyserial > /dev/null 2>&1

for sketch in $(find ./Firmware/ -type f -name "*.ino"); do
    filename=$(basename "$sketch")  # Extract only the .ino filename
    sketch_dir=$(dirname "$sketch")  # Get the directory of the .ino file
    config_file="$sketch_dir/.fqbn"  # Path to the .fqbn file in the same directory
    echo "🗂️ Compiling $filename"

    # Check if the .fqbn file exists
    if [ -f "$config_file" ]; then
        echo "🔎 Searching for .fqbn in $sketch_dir"
        config_content=$(head -n 1 "$config_file")
    else
        echo "🚫 Didn't find build parameter in $sketch_dir"
        echo "------------------------"
        continue
    fi
    echo "🖥️ Compiling with configuration: $config_content"
    # Compile the .ino file
    if arduino-cli compile -b $config_content "$sketch" > /dev/null 2>&1; then
        echo "✅ Compilation successful for $filename"
    else
        echo "❌ Compilation error for $filename"
        arduino-cli compile -b $config_content "$sketch" 2>&1 | sed -E 's|(/[^ ]+/)+||g'
    fi
    echo "------------------------"
    config_content=""
done
deactivate
