/**
 * Cli.re
 *
 * Module for handling command-line arguments for Oni2
 */
open Rench;

type t = {
  folder: string,
  filesToOpen: list(string),
};

let newline = "\n";

let header = (version: string) => {
  newline ++ "Onivim 2." ++ version ++ newline ++ newline;
};

let show = (v: t) => {
  let files =
    List.fold_left((curr, p) => curr ++ newline ++ p, "", v.filesToOpen);

  "Folder: " ++ v.folder ++ newline ++ "Files: " ++ files;
};

let noop = () => ();

let parse = (setup: Setup.t) => {
  let args: ref(list(string)) = ref([]);

  Arg.parse([
    ("-attach", Unit(noop), "")
], arg => args := [arg, ...args^], header(setup.version));

  let paths = args^ |> List.rev;
  let workingDirectory = Environment.getWorkingDirectory();

  let stripTrailingPathCharacter = s => {
    let len = String.length(s);
    if (len >= 1 && s.[len - 1] == '/') {
      String.sub(s, 0, len - 1);
    } else {
      s;
    };
  };

  let isAbsolutePathWithTilde = s =>
    switch (s.[0]) {
    | '~' => !Sys.win32
    | _ => false
    | exception (Invalid_argument(_)) => false
    };

  let resolvePath = p => {
    let p =
      if (Path.isAbsolute(p) || isAbsolutePathWithTilde(p)) {
        p;
      } else {
        Path.join(workingDirectory, p);
      };

    p |> Path.normalize |> stripTrailingPathCharacter;
  };

  let absolutePaths = List.map(resolvePath, paths);

  let isDirectory = p =>
    switch (Sys.is_directory(p)) {
    | v => v
    | exception (Sys_error(_)) => false
    };

  let directories = List.filter(isDirectory, absolutePaths);
  let filesToOpen = List.filter(p => !isDirectory(p), absolutePaths);

  let folder =
    switch (directories) {
    | [] => workingDirectory
    | [hd, ..._] => hd
    };

  {folder, filesToOpen};
};
