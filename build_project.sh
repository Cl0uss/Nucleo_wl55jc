#!/usr/bin/env bash
set -e

cd ~/zephyr/Nucleo_wl55jc

# searching for all subfolders except build and .vscode
projects=( $(find . -maxdepth 1 -type d ! -name ".*" ! -name "build" -printf "%f\n" | sort) )

# if nothing was found
if [ ${#projects[@]} -eq 0 ]; then
  echo "❌ Нет доступных проектов в $(pwd)"
  exit 1
fi

# displaying list
echo "📦 Found projects:"
for i in "${!projects[@]}"; do
  echo "$((i+1))) ${projects[$i]}"
done

# asking for choice
echo -n "Enter projects number: "
read -r choice

# checking choice
if ! [[ "$choice" =~ ^[0-9]+$ ]] || [ "$choice" -lt 1 ] || [ "$choice" -gt "${#projects[@]}" ]; then
  echo "❌ Wrong number."
  exit 1
fi

project="${projects[$((choice-1))]}"
echo "🚀 Project build: $project"

# activating .venv
source ~/zephyr/.venv/bin/activate

# west build and flash
cd "$project"
west build --pristine=always -b nucleo_wl55jc && west flash
