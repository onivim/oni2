open Oni_Core;
open Oni_Neovim;
open Rench;
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

describe("NeovimApi", ({describe, test, _}) => {
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

    withNeovimApi(api =>
      Helpers.repeat(
        10,
        () => {
          let result =
            api.requestSync(
              "nvim__id",
              Msgpck.List([
                Msgpck.List([
                  Msgpck.String("hey, neovim!"),
                  Msgpck.String("hey again, neovim!"),
                ]),
              ]),
            );

          switch (result) {
          | Msgpck.List([Msgpck.String(msg1), Msgpck.String(msg2)]) =>
            expect.string(msg1).toEqual("hey, neovim!");
            expect.string(msg2).toEqual("hey again, neovim!");
          | _ => expect.string("FAIL").toEqual("")
          };
          count := count^ + 1;
        },
      )
    );
  });

  describe("ui_attach", ({test, _}) =>
    test("basic ui_attach / ui_detach", ({expect}) =>
      Helpers.repeat(10, () =>
        withNeovimApi(api => {
          let notifications: ref(list(NeovimApi.notification)) = ref([]);

          let _ =
            Event.subscribe(api.onNotification, n =>
              notifications := List.append([n], notifications^)
            );

          let _result =
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

          let f = () => {
            api.pump();
            List.length(notifications^) > 0;
          };

          Utility.waitForCondition(f);

          let s = (n: NeovimApi.notification) => {
            prerr_endline("NOTIFICATION: " ++ Msgpck.show(n.payload));
          };

          List.iter(s, notifications^);

          expect.bool(List.length(notifications^) >= 1).toBe(true);
        })
      )
    )
  );
});
