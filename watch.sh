#!/bin/bash

# Function to clean up background processes on exit
cleanup() {
  echo "Cleaning up..."
  kill $(jobs -p) 2>/dev/null
  exit 0
}

# Set up trap for cleanup on script termination
trap cleanup SIGINT SIGTERM

echo "👀 Watching for changes in source files..."

# Function to get file checksums
get_checksums() {
  find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "CMakeLists.txt" -o -name "*.sh" \) \
    -not -path "./build/*" \
    -exec stat -f "%N %m" {} \;
}

# Initial build
./dev.sh &
last_pid=$!

# Store initial state
last_checksums=$(get_checksums)

while true; do
  sleep 1
  current_checksums=$(get_checksums)

  if [[ "$current_checksums" != "$last_checksums" ]]; then
    # Find changed files
    changed_files=$(diff <(echo "$last_checksums") <(echo "$current_checksums") | grep ">" | cut -d' ' -f2-)
    echo "🔍 Changes detected in: $changed_files"

    # Kill previous build
    if [[ -n "$last_pid" ]]; then
      kill $last_pid 2>/dev/null
      wait $last_pid 2>/dev/null
    fi

    # Clear screen and rebuild
    clear
    echo "🔄 Rebuilding..."

    # Start new build
    ./dev.sh &
    last_pid=$!

    # Update checksums
    last_checksums="$current_checksums"
  fi
done
