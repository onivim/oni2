#!/bin/bash

# Set the path for the file we are going to generate
config_path="$(pwd)/assets/configuration"
OUTPUT="$config_path/setup.json"

# Figure out what OS we are on
current_os="$(uname -s)"
case "${current_os}" in
  Linux*)     machine=Linux;;
  Darwin*)    machine=Mac;;
  CYGWIN*)    machine=Cygwin;;
  MINGW*)     machine=MinGw;;
  *)          machine="UNKNOWN:${current_os}"
esac

check_command_succeded() {
  # $? variable is the return code of a command
  if [ $? -eq 0 ]; then
    echo "Command: $1 Succeded"
  else
    echo "Command: $1 Failed"
    exit
  fi
}

# this is a utility to help adding more variables to the generated file
append_line() {
  echo "$1" >> "$OUTPUT"
}

# Based on the operating system get the correct vendor path
# for the Oni2 binary
case "${machine}" in
  Linux)
      NODE_PATH="$(pwd)/vendor/node-v10.15.1/linux-x64/node"
      TEXTMATE_SERVICE_PATH="$(pwd)/src/textmate_service/lib/src/index.js"
      EXTENSIONS_PATH="$(pwd)/extensions"
      NEOVIM_PATH="$(pwd)/vendor/neovim-0.3.3/nvim-linux64/bin/nvim"
      CONFIGURATION_PATH="$(pwd)/assets/configuration/configuration.json";;
  Mac)
      NODE_PATH="$(pwd)/vendor/node-v10.15.1/osx/node"
      TEXTMATE_SERVICE_PATH="$(pwd)/src/textmate_service/lib/src/index.js"
      EXTENSIONS_PATH="$(pwd)/extensions"
      NEOVIM_PATH="$(pwd)/vendor/neovim-0.3.3/nvim-osx64/bin/nvim"
      CONFIGURATION_PATH="$(pwd)/assets/configuration/configuration.json";;
  *)
      NEOVIM_PATH="$(pwd)/vendor/neovim-0.3.3/nvim-win64/bin/nvim.exe"
      NEOVIM_PATH="$(cygpath -m "$NEOVIM_PATH")"
      EXTENSIONS_PATH="$(pwd)/extensions"
      EXTENSIONS_PATH="$(cygpath -m "$EXTENSIONS_PATH")"
      TEXTMATE_SERVICE_PATH="$(pwd)/src/textmate_service/lib/src/index.js"
      TEXTMATE_SERVICE_PATH="$(cygpath -m "$TEXTMATE_SERVICE_PATH")"
      NODE_PATH="$(pwd)/vendor/node-v10.15.1/win-x64/node.exe"
      NODE_PATH="$(cygpath -m "$NODE_PATH")"
      CONFIGURATION_PATH="$(pwd)/assets/configuration/configuration.json"
      CONFIGURATION_PATH="$(cygpath -m "$CONFIGURATION_PATH")";;
esac

oni_bin_path="{neovim:\"$NEOVIM_PATH\",node:\"$NODE_PATH\",configuration:\"$CONFIGURATION_PATH\",textmateService:\"$TEXTMATE_SERVICE_PATH\",bundledExtensions:\"$EXTENSIONS_PATH\"}"

# create the current bin path as this might not exist yet
if [ ! -d "$config_path" ]; then
  mkdir -p "$config_path"
  check_command_succeded "creating parent directory: $config_path"
fi
# create the output file, if it exists remove it first so it is recreated
if [[ -e "$OUTPUT" ]]; then
  rm -f "$OUTPUT"
  check_command_succeded "removing old setup $OUTPUT"
fi

touch "$OUTPUT"
check_command_succeded "creating new $OUTPUT"

append_line "$oni_bin_path"
