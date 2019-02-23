/*
 * NeovimProtocol.re
 *
 * Wrapper on-top of the NeovimApi primitives to hide the Msgpack details.
 */

open Oni_Core.Types;
open Rench;

/*
 * Simple API for interacting with Neovim,
 * abstracted from Msgpck
 */
module M = Msgpck;

type t = {
  uiAttach: unit => unit,
  input: string => unit,
  bufAttach: int => unit,
  openFile:
    (~path: string=?, ~bufferId: int=?, ~openMethod: openMethod=?, unit) =>
    unit,
  /* TODO */
  /* Typed notifications */
  onNotification: Event.t(Notification.t),
};

let make = (nvimApi: NeovimApi.t) => {
  let onNotification: Event.t(Notification.t) = Event.create();

  let uiAttach = () => {
    let _ =
      nvimApi.requestSync(
        "nvim_ui_attach",
        M.List([
          M.Int(20),
          M.Int(20),
          M.Map([
            (M.String("rgb"), M.Bool(true)),
            (M.String("ext_popupmenu"), M.Bool(true)),
            (M.String("ext_tabline"), M.Bool(true)),
            (M.String("ext_cmdline"), M.Bool(true)),
            (M.String("ext_wildmenu"), M.Bool(true)),
            (M.String("ext_linegrid"), M.Bool(true)),
            /* (M.String("ext_multigrid"), M.Bool(true)), */
            /* (M.String("ext_hlstate"), M.Bool(true)), */
          ]),
        ]),
      );
    ();
  };

  let input = (key: string) =>
    nvimApi.requestSync("nvim_input", M.List([M.String(key)])) |> ignore;

  let bufAttach = id => {
    let _error =
      nvimApi.requestSync(
        "nvim_buf_attach",
        M.List([M.Int(id), M.Bool(true), M.Map([])]),
      );
    ();
  };

  Event.subscribe(
    nvimApi.onNotification,
    n => {
      let parsedEvent = Notification.parse(n.notificationType, n.payload);

      let f = n => Event.dispatch(onNotification, n);

      List.iter(f, parsedEvent);
    },
  )
  |> ignore;

  let openFile = (~path=?, ~bufferId=?, ~openMethod=Buffer, ()) => {
    let args =
      switch (path, bufferId, openMethod) {
      | (Some(p), None, Buffer) => M.List([M.String("edit" ++ " " ++ p)])
      | (None, Some(id), Buffer) =>
        /*
         The # is required to open a buffer by id using the `edit` command
         */
        M.List([M.String("edit #" ++ string_of_int(id))])
      | _ => M.List([])
      };

    let _error = nvimApi.requestSync("nvim_command", args);
    ();
  };

  let ret: t = {uiAttach, input, onNotification, bufAttach, openFile};
  ret;
};
