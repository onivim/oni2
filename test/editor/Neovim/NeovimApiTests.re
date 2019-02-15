open Oni_Core;
open Oni_Neovim;
open Rench;
open TestFramework;

open Helpers;

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
      repeat(() => {
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
      })
    );
  });

  describe("ui_attach", ({test, _}) =>
    test("basic ui_attach / ui_detach", ({expect}) =>
      repeat(() =>
        withNeovimApi(api => {
          let notifications: ref(list(NeovimApi.notification)) = ref([]);

          let _ =
            Event.subscribe(api.onNotification, n =>
              notifications := List.append([n], notifications^)
            );

          let _result = uiAttach(api);

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

  describe("basic buffer edit", ({test, _}) =>
    test("modify lines in buffer", ({expect}) =>
      repeat(() =>
        withNeovimApi(api => {
          let notifications: ref(list(NeovimApi.notification)) = ref([]);

          let _ =
            Event.subscribe(api.onNotification, n =>
              notifications := List.append([n], notifications^)
            );

          let _result = uiAttach(api);

          let _response =
            api.requestSync(
              "nvim_input",
              Msgpck.List([Msgpck.String("iabcd<CR>efghi")]),
            );

          let lines =
            api.requestSync("nvim_get_current_line", Msgpck.List([]));

          switch (lines) {
          | Msgpck.String(s) => expect.string(s).toEqual("efghi")
          | _ => expect.string("fail").toEqual("to get proper input")
          };
        })
      )
    )
  );
});
