// MODEL

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
  let getCompletionMeet = (~byteIndex, commandLine) => {
    let len = String.length(commandLine);

    if (len == 0 || byteIndex < len) {
      None;
    } else {
      /* Look backwards for '/' or ' ' */
      let found = ref(false);
      let meet = ref(byteIndex);

      while (meet^ > 0 && ! found^) {
        let pos = meet^ - 1;
        let c = commandLine.[pos];
        if (c == ' ') {
          found := true;
        } else {
          decr(meet);
        };
      };

      Some(meet^);
    };
  };

  let%test "empty command line returns None" = {
    getCompletionMeet(~byteIndex=0, "") == None;
  };

  let%test "meet before command" = {
    getCompletionMeet(~byteIndex=3, "vsp") == Some(0);
  };

  let%test "meet after command" = {
    getCompletionMeet(~byteIndex=4, "vsp ") == Some(4);
  };

  let%test "meet with a path, no spaces" = {
    getCompletionMeet(~byteIndex=4, "vsp /path/") == Some(4);
  };

  let%test "meet with a path, spaces" = {
    getCompletionMeet(~byteIndex=4, "vsp /path with spaces/") == Some(4);
  };
};
