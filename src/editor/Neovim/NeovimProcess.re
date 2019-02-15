/*
 * NeovimProcess.re
 *
 * Management of running `nvim` processes
 */

open Rench;
open Oni_Core;

type t = {pid: int};

exception NeovimNotFound(string);

let extractParts = line => {
  let parts = Str.split(Str.regexp("="), line);
  switch (parts) {
  | [name, value] => Some((name, value))
  | _ => None
  };
};

let getNeovimPathFromCandidates = paths => {
  List.find(
    item =>
      switch (item) {
      | Some(("ONI2_PATH", _)) => true
      | _ => false
      },
    paths,
  )
  |> (
    path =>
      switch (path) {
      | Some((_, p)) => p
      | None => raise(NeovimNotFound("Neovim binary could not be found"))
      }
  );
};

let getNeovimPath = () => {
  let setupFilePath = Environment.getExecutingDirectory() ++ "/setup.txt";
  Utility.getFileContents(setupFilePath, ~handler=extractParts)
  |> getNeovimPathFromCandidates;
};

let version = () => {
  let neovimPath = getNeovimPath();
  let ret = ChildProcess.spawnSync(neovimPath, [|"--version"|]);
  ret.stdout;
};

let start = (~args: array(string)) => {
  let neovimPath = getNeovimPath();
  print_endline("Starting oni from binary path: " ++ neovimPath);
  ChildProcess.spawn(neovimPath, args);
};
