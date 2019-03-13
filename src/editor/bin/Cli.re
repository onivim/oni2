/**
 * Cli.re
 *
 * Module for handling command-line arguments for Oni2
 */

open Oni_Core;

open Rench;

type t = {
    folder: string,
    filesToOpen: list(string),
};

let newline = "\n"

let header = (version: string) => {
    newline
    ++ "Onivim 2." ++ version ++ newline
    ++ newline
};

let show = (v: t) => {
    let files = List.fold_left((curr, p) => curr ++ newline ++ p, "", v.filesToOpen);

    "Folder: " ++ v.folder ++ newline
    ++ "Files: " ++  newline ++ files;
}

let parse = (setup: Setup.t) => {
  let args: ref(list(string)) = ref([]);

  Arg.parse([], (arg) => {
      args := [arg, ...args^];
  }, header(setup.version));


  let paths = args^ |> List.rev;
  let workingDirectory = Rench.Environment.getWorkingDirectory();

  let resolvePath = p => {
    if (Rench.Path.isAbsolute(p)) {
        p
    } else {
        Rench.Path.join(workingDirectory, p);
    }
  };

  let absolutePaths = List.map(resolvePath, paths);

  let isDirectory = p => switch(Sys.is_directory(p)) {
  | v => v
  | Sys_error(x) => false
  };

  let directories = List.filter(isDirectory, absolutePaths);
  let filesToOpen = List.filter((p) => !isDirectory(p), absolutePaths);

  let folder = switch(directories) {
  | [] => workingDirectory
  | [hd, ..._] => hd
  };

  { folder, filesToOpen };
};
