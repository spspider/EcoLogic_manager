#!/usr/bin/env bash

source_dir="HTML_data"
destination_dir="EcoLogic_manager/data"

mkdir -p "$destination_dir"

file_list=(
  "condition.htm"
  "edit.htm"
  "graphs.htm"
  "help.htm"
  "home.htm"
  "index.htm"
  "IR_setup.htm"
  "other_setup.htm"
  "pin_setup.htm"
  "wifi_setup.htm"
  "ws2811.html"
  "scripts/ace.min.js"
  "scripts/chart.min.js"
  "scripts/condition.js"
  "scripts/graphs.js"
  "scripts/help.js"
  "scripts/helper_func.js"
  "scripts/home_ir.js"
  "scripts/IR_recieve.js"
  "scripts/other_setup.js"
  "scripts/pin_setup.js"
  "scripts/script_home.js"
  "scripts/style_generated.css"
  "scripts/wifi_setup.js"
  "scripts/ws2811.js"
  "scripts/ws2812_set.js"
  "homeassistant.htm"
  "scripts/homeassistant.js"
)

for name in "${file_list[@]}"; do
  src="$source_dir/$name"
  dest="$destination_dir/$name.gz"
  dest_dir=$(dirname "$dest")

  if [ ! -f "$src" ]; then
    echo "Skipping missing file: $src"
    continue
  fi

  mkdir -p "$dest_dir"
  gzip -9 -c "$src" > "$dest"
  echo "Created: $dest"
done
