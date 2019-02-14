/*
 * NeovimProcess.re
 *
 * Management of running `nvim` processes
 */

open Rench;
open Oni_Core;

type t = {pid: int};

let version = (~neovimPath: string) => {
  let ret = ChildProcess.spawnSync(neovimPath, [|"--version"|]);
  ret.stdout;
};

let extractParts = line => {
  let parts = Str.split(Str.regexp("="), line);
  switch (parts) {
  | [name, value] => Some((name, value))
  | _ => None
  };
};

let getNeovimPath = paths => {
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
      | None => raise(Not_found)
      }
  );
};

let start = (~args: array(string)) => {
  let setupFilePath = Environment.getExecutingDirectory() ++ "/setup.txt";
  let neovimPath =
    Utility.getFileContents(setupFilePath, ~handler=extractParts)
    |> getNeovimPath;
  print_endline("Starting oni from binary path: " ++ neovimPath);
  ChildProcess.spawn(neovimPath, args);
};
