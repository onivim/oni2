open Oni_Neovim;

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

let withNeovimApi = f => {
  let nvim = NeovimProcess.start(~args=[|"--embed"|]);
  let msgpackTransport =
    MsgpackTransport.make(
      ~onData=nvim.stdout.onData,
      ~write=nvim.stdin.write,
      (),
    );
  let nvimApi = NeovimApi.make(msgpackTransport);

  f(nvimApi);

  msgpackTransport.close();
};

let withNeovimProtocol = f => {
    withNeovimApi((nvimApi) => {
        let neovimProtocol = NeovimProtocol.make(nvimApi);  
        let _ = uiAttach(nvimApi);
        f((nvimApi, neovimProtocol));
    });
}
