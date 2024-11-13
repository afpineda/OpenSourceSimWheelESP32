#!/bin/sh
# Required dependencies:
# - pip
# - python
# - arduino-cli

#Installing dependencies
arduino-cli config init --additional-urls https://downloads.arduino.cc/packages/package_index.json> /dev/null 2>&1 && \
arduino-cli core update-index && \
arduino-cli core install esp32:esp32

#Preparation to compile (Making Symsliks, activating python virtual enviroment , installing pyserial)
./MakeSymLinks.sh > /dev/null 2>&1
python -m venv . > /dev/null 2>&1
source ./bin/activate 
pip install pyserial > /dev/null 2>&1

for sketch in $(find ./Firmware/ -type f -name "*.ino"); do
    filename=$(basename "$sketch")  # Extract only the .ino filename
    sketch_dir=$(dirname "$sketch")  # Get the directory of the .ino file
    config_file="$sketch_dir/.fqbn"  # Path to the .fqbn file in the same directory
    echo "Compiling $filename"

    # Check if the .fqbn file exists
    if [ -f "$config_file" ]; then
        echo "🔎 Searching for .fqbn in $sketch_dir"
        config_content=$(cat "$config_file")  # Save the content of the .fqbn file
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
    fi
    echo "------------------------"
    config_content=""
done
deactivate
