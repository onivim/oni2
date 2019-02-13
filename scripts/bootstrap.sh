# Set the path for the file we are going to generate
OUTPUT="$(pwd)/setup.txt"

# Figure out what OS we are on
current_os="$(uname -s)"
case "${current_os}" in
  Linux*)     machine=Linux;;
  Darwin*)    machine=Mac;;
  CYGWIN*)    machine=Cygwin;;
  MINGW*)     machine=MinGw;;
  *)          machine="UNKNOWN:${current_os}"
esac

# this is a utility to help adding more variables to the generated file
append_line() {
  echo $1 >> $OUTPUT
}

# Based on the operating system get the correct vendor path
# for the Oni2 binary
case "${machine}" in
  Linux)
      ONI_PATH="$(pwd)/vendor/neovim-0.3.3/nvim-linux64/bin/nvim";;
  Mac)
      ONI_PATH="$(pwd)/vendor/neovim-0.3.3/nvim-osx64/bin/nvim";;
  *)
      ONI_PATH="$(pwd)/vendor/neovim-0.3.3/nvim-win64/bin/nvim.exe"
      ONI_PATH="$(cygpath -m $ONI_PATH)";;
esac

oni_bin_path="ONI2_PATH=$ONI_PATH"

# create the output file, if it exists remove it first so it is recreated
if [[ -e $OUTPUT ]]; then
  rm -f $OUTPUT
fi

touch $OUTPUT

append_line $oni_bin_path
