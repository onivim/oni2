open Oni_Core.Utility;

type context = {
  output: string,
  status: Unix.process_status,
};

module Internal = {
  let statusToString =
    fun
    | Unix.WEXITED(code) => Printf.sprintf("WEXITED: %d", code)
    | Unix.WSIGNALED(code) => Printf.sprintf("WSIGNALED: %d", code)
    | Unix.WSTOPPED(code) => Printf.sprintf("WSTOPPED: %d", code);

  let start = (~executable, ~args) => {
    let executingDirectory = Revery.Environment.getExecutingDirectory();

    let executablePath = Rench.Path.join(executingDirectory, executable);

    prerr_endline("Running executable: " ++ executablePath);
    args |> List.iter(prerr_endline);

    let input =
      Unix.open_process_args_in(
        executablePath,
        [executablePath, ...args] |> Array.of_list,
      );

    let lines = ref([]);

    let readLines = () => {
      while (true) {
        let line = input_line(input);
        lines := [line, ...lines^];
      };
    };

    let (lines, status) =
      try(
        {
          readLines();
          let status = Unix.close_process_in(input);
          ([], status);
        }
      ) {
      | End_of_file =>
        let status = Unix.close_process_in(input);
        (List.rev(lines^), status);
      };

    let output = String.concat("\n", lines);

    prerr_endline("Got output: |" ++ output ++ "|");
    prerr_endline("Status: " ++ statusToString(status));

    {output, status};
  };
};

let launcherExecutable = Sys.win32 ? "Oni2.exe" : "Oni2";
let editorExecutable = Sys.win32 ? "Oni2_editor.exe" : "Oni2_editor";

let startWithArgs = (args: list(string)) => {
  Internal.start(~executable=launcherExecutable, ~args);
};

let startEditorWithArgs = (args: list(string)) => {
  Internal.start(~executable=editorExecutable, ~args);
};

let validateOutputContains = (query, {output, _} as context) => {
  if (!StringEx.contains(query, output)) {
    prerr_endline("Expected output to contain: " ++ query);
    prerr_endline("But received: " ++ output);
    failwith("Output incorrect");
  };

  context;
};

let validateOutputDoesNotContain = (query, {output, _} as context) => {
  if (StringEx.contains(query, output)) {
    prerr_endline("Expected output to not contain: " ++ query);
    prerr_endline("But received: " ++ output);
    failwith("Output incorrect");
  };

  context;
};

let validateExitStatus = (exitStatus, {status, _} as context) => {
  if (exitStatus != status) {
    prerr_endline(
      "Expected status to be: " ++ Internal.statusToString(exitStatus),
    );
    prerr_endline("But received: " ++ Internal.statusToString(status));
    failwith("Exit code incorrect");
  };

  context;
};

let finish = _context => {
  (); // TODO:
    // Check for hanging / rogue processes?
};
