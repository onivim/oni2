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
  openFile: Views.viewOperation,
  closeFile: Views.viewOperation,
  /* TODO */
  /* Typed notifications */
  onNotification: Event.t(Notification.t),
};

type commands = {
  bufferPath: string => M.t,
  bufferId: int => M.t,
  tabId: int => M.t,
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

  /**
     An abstraction/helper function that deals with the various
     ways that you can interact with nvim views either buffers or tabs
     each takes their own commands.

     This way you can create a function that can be used to interact
     with either depending on what a user passes in, not it one OR the
     other depending on the open mode that the caller decides on
   */
  let viewOperationCommand =
      (commands, ~path=?, ~id=?, ~openMethod: Views.openMethod=Buffer, ()) => {
    let args =
      switch (path, id, openMethod) {
      | (Some(p), None, Buffer) => commands.bufferPath(p)
      | (None, Some(id), Buffer) => commands.bufferId(id)
      | (None, Some(id), Tab) => commands.tabId(id)
      | _ => M.List([])
      };

    let _error = nvimApi.requestSync("nvim_command", args);
    ();
  };

  let openFile =
    viewOperationCommand({
      bufferPath: p => M.List([M.String("edit " ++ p)]),
      bufferId: id =>
        /*
         The # is required to open a buffer by id using the `edit` command
         */
        M.List([M.String("edit #" ++ string_of_int(id))]),
      tabId: id => M.List([M.String("tabedit " ++ string_of_int(id))]),
    });

  let closeFile =
    viewOperationCommand({
      bufferId: id => M.List([M.String("bd! " ++ string_of_int(id))]),
      bufferPath: _p => M.Nil,
      tabId: id => M.List([M.String("tabdelete! " ++ string_of_int(id))]),
    });

  let ret: t = {
    uiAttach,
    input,
    onNotification,
    bufAttach,
    openFile,
    closeFile,
  };
  ret;
};
