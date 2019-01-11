open Rench;

open Oni_Neovim;

exception EnvironmentVariableNotFound;

let optOrThrow = (s: option(string)) => {
  switch (s) {
  | Some(v) => v
  | _ => raise(EnvironmentVariableNotFound)
  };
};

let getNeovimPath = () =>
  Environment.getEnvironmentVariable("ONI2_NEOVIM_PATH") |> optOrThrow;

let repeat = (times: int, f) => {
  let count = ref(0);

  while (count^ < times) {
    f();
    count := count^ + 1;
  };
};

let uiAttach = (api: NeovimApi.t) => {
  api.requestSync(
    "nvim_ui_attach",
    Msgpck.List([
      Msgpck.Int(20),
      Msgpck.Int(20),
      Msgpck.Map([
        (Msgpck.String("rgb"), Msgpck.Bool(true)),
        (Msgpck.String("ext_popupmenu"), Msgpck.Bool(true)),
        (Msgpck.String("ext_tabline"), Msgpck.Bool(true)),
        (Msgpck.String("ext_cmdline"), Msgpck.Bool(true)),
        (Msgpck.String("ext_wildmenu"), Msgpck.Bool(true)),
        (Msgpck.String("ext_linegrid"), Msgpck.Bool(true)),
        /* (Msgpck.String("ext_multigrid"), Msgpck.Bool(true)), */
        /* (Msgpck.String("ext_hlstate"), Msgpck.Bool(true)), */
      ]),
    ]),
  );
};
