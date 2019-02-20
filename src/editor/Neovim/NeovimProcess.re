/*
 * NeovimProcess.re
 *
 * Management of running `nvim` processes
 */

open Rench;

type t = {pid: int};

let version = (~neovimPath: string) => {
  let ret = ChildProcess.spawnSync(neovimPath, [|"--version"|]);
  ret.stdout;
};

let start = (~neovimPath, ~args: array(string)) => {
  print_endline("Starting oni from binary path: " ++ neovimPath);
  ChildProcess.spawn(neovimPath, args);
};
