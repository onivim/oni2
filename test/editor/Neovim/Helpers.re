open Rench;

open Oni_Neovim;
open TestFramework;

/* open Helpers; */

exception EnvironmentVariableNotFound;

let optOrThrow = (s: option(string)) => {
  switch (s) {
  | Some(v) => v
  | _ => raise(EnvironmentVariableNotFound)
  };
};

let getNeovimPath = () => 
    Environment.getEnvironmentVariable("ONI2_NEOVIM_PATH")
    |> optOrThrow;
