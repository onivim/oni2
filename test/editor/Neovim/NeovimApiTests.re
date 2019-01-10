open Oni_Neovim;
open TestFramework;

let withNeovimApi = f => {
  let neovimPath = Helpers.getNeovimPath();
  let nvim = NeovimProcess.start(~neovimPath, ~args=[|"--embed"|]);
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

describe("NeovimApi", ({test, _}) => {
  test("nvim__id", ({expect}) =>
    withNeovimApi(api => {
      let result =
        api.requestSync(
          "nvim__id",
          Msgpck.List([Msgpck.String("hey, neovim!")]),
        );

      switch (result) {
      | Msgpck.String(msg) => expect.string(msg).toEqual("hey, neovim!")
      | _ => expect.string("FAIL").toEqual("")
      };
    })
  );

  test("nvim__id_array", ({expect}) => {

    let count = ref(0);


    withNeovimApi(api => {
        Helpers.repeat(1, () => {

          let attach =
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
                    ])
                  ]));

          prerr_endline ("ATTACH: " ++ Msgpck.show(attach));

          /* let derp = ref(0); */
          /* while (derp^  < 20) { */
            Unix.sleepf(0.1);
          /*   derp := derp^ + 1; */
          /* }; */

          /* prerr_endline ("starting put..."); */
          /* let _result = */
          /*   api.requestSync( */
          /*     "nvim_command_output", */
          /*     Msgpck.List([ */
          /*         Msgpck.String(":echo 'Hello!'"), */
          /*     ]), */
          /*   ); */

          /* prerr_endline ("ending put..." ++ Msgpck.show(_result)); */
          let result = Msgpck.List([]);

          /* let input = api.requestSync( */
          /*   "nvim_input", */
          /*   Msgpck.List([Msgpck.String("iabc")]) */
          /* ); */
          /* prerr_endline ("INPUT: " ++ Msgpck.show(input)); */
          /* let derp = ref(0); */
          /* while (derp^  < 20) { */
          /*   Unix.sleepf(0.11); */
          /*   derp := derp^ + 1; */
          /* }; */

          /* let line = api.requestSync( */
          /*   "nvim_list_runtime_paths", */
          /*   Msgpck.List([]), */
          /* ); */
          /* prerr_endline ("LINE: " ++ Msgpck.show(line)); */

          /* let line2 = api.requestSync( */
          /*   "nvim_list_uis", */
          /*   Msgpck.List([]), */
          /* ); */
          /* prerr_endline ("UIs: " ++ Msgpck.show(line2)); */

          switch (result) {
          | Msgpck.List([Msgpck.String(msg1), Msgpck.String(msg2)]) =>
            expect.string(msg1).toEqual("hey, neovim!");
            expect.string(msg2).toEqual("hey again, neovim!");
          | _ => {
              prerr_endline ("MSGPCK: " ++ Msgpck.show(result));
              expect.string("FAIL").toEqual("FAIL");
          };
          };
          count := count^ + 1;
        });
    });
    });
});
