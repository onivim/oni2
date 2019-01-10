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
        Helpers.repeat(100, () => {

          let attach =
              api.requestSync(
                  "nvim_ui_attach",
                  Msgpck.List([
                    Msgpck.Int(10),
                    Msgpck.Int(10),
                    Msgpck.Map([])
                  ]));
          prerr_endline ("ATTACH: " ++ Msgpck.show(attach));


          let result =
            api.requestSync(
              "nvim_input",
              Msgpck.List([
                  Msgpck.String("iabc<CR>"),
              ]),
            );

          let line = api.requestSync(
            "nvim_get_current_line",
            Msgpck.List([]),
          );
          prerr_endline ("LINE: " ++ Msgpck.show(line));

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
