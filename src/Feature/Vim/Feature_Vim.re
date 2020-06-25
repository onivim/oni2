// MODEL
open Oni_Core.Utility;

type model = {mode: Vim.Mode.t};

let initial = {mode: Vim.Types.Normal};

let mode = ({mode}) => mode;

// MSG

[@deriving show]
type msg =
  | ModeChanged([@opaque] Vim.Mode.t);

let update = (msg, _model: model) => {
  switch (msg) {
  | ModeChanged(mode) => ({mode: mode}: model)
  };
};

module CommandLine = {
  let getCompletionMeet = commandLine => {
    let len = String.length(commandLine);

    if (len == 0) {
      None;
    } else {
      String.index_opt(commandLine, ' ')
      |> Option.map(idx => idx + 1)  // Advance past space
      |> OptionEx.or_(Some(0));
    };
  };

  let%test "empty command line returns None" = {
    getCompletionMeet("") == None;
  };

  let%test "meet before command" = {
    getCompletionMeet("vsp") == Some(0);
  };

  let%test "meet after command" = {
    getCompletionMeet("vsp ") == Some(4);
  };

  let%test "meet with a path, no spaces" = {
    getCompletionMeet("vsp /path/") == Some(4);
  };

  let%test "meet with a path, spaces" = {
    getCompletionMeet("vsp /path with spaces/") == Some(4);
  };
};
