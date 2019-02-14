# Set the path for the file we are going to generate
OUTPUT="$cur__bin/setup.txt"

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
  if [ $? -eq 0 ]; then
    echo Command: $1 Succeded
  else
    echo Command: $1 Failed
    exit
  fi
}

# this is a utility to help adding more variables to the generated file
append_line() {
  echo $1 >> $OUTPUT
}

# Based on the operating system get the correct vendor path
# for the Oni2 binary
case "${machine}" in
  Linux)
      ONI_PATH="$cur__bin/vendor/neovim-0.3.3/nvim-linux64/bin/nvim";;
  Mac)
      ONI_PATH="$cur__bin/vendor/neovim-0.3.3/nvim-osx64/bin/nvim";;
  *)
      ONI_PATH="$cur__bin/vendor/neovim-0.3.3/nvim-win64/bin/nvim.exe"
      ONI_PATH="$(cygpath -m $ONI_PATH)";;
esac

oni_bin_path="ONI2_PATH=$ONI_PATH"

# create the current bin path as this might not exist yet
if [ ! -d "$cur__bin" ]; then
  mkdir -p $cur__bin
  check_command_succeded "creating parent directory: $cur__bin"
fi
# create the output file, if it exists remove it first so it is recreated
if [[ -e $OUTPUT ]]; then
  rm -f $OUTPUT
  check_command_succeded "removing old setup $OUTPUT"
fi

touch $OUTPUT
check_command_succeded "creating new $OUTPUT"

append_line $oni_bin_path
