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

let extractParts = line => {
  let parts = Str.split(Str.regexp("="), line);
  switch (parts) {
  | [name, value] => (name, value)
  | _ => ("", "")
  };
};

let getOniPath = paths => {
  let (_, path) =
    List.find(
      ((name, _)) =>
        switch (name) {
        | "ONI2_PATH" => true
        | _ => false
        },
      paths,
    );
  path;
};

let getBuildVariablesFromSetup = (~path="/setup.txt", ()) => {
  let variableList = ref([]);

  let fileInChannel =
    Pervasives.open_in(Environment.getWorkingDirectory() ++ path);

  let fileStream =
    Stream.from(_i =>
      switch (Pervasives.input_line(fileInChannel)) {
      | line => Some(line)
      | exception End_of_file => None
      }
    );
  fileStream
  |> Stream.iter(line => {
       let parts = extractParts(line);
       variableList := [parts, ...variableList^];
     });

  variableList^;
};

let start = (~neovimPath as _: string, ~args: array(string)) => {
  let variables = getBuildVariablesFromSetup();
  let neovimPath = getOniPath(variables);
  print_endline("Starting oni from binary path: " ++ neovimPath);
  ChildProcess.spawn(neovimPath, args);
};
