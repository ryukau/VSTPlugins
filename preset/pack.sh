#!/bin/bash

for preset_dir in Uhhyou/*; do
  echo "$preset_dir"
  zip -r "$preset_dir"Presets.zip "$preset_dir"
done
