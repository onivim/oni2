/*
 * NeovimProtocol.re
 *
 * Wrapper on-top of the NeovimApi primitives to hide the Msgpack details.
 */

/* open Oni_Core; */
open Rench;

/*
 * Simple API for interacting with Neovim,
 * abstracted from Msgpck
 */
type t = {
  uiAttach: unit => unit,
  input: string => unit,
  bufAttach: int => unit,
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
    ();
  };

  let input = (key: string) => {
    let _ =
      nvimApi.requestSync("nvim_input", Msgpck.List([Msgpck.String(key)]));
    ();
  };

  let bufAttach = id => {
    let _ =
      nvimApi.requestSync(
        "nvim_buf_attach",
        Msgpck.List([Msgpck.Int(id), Msgpck.Bool(true), Msgpck.Map([])]),
      );
    ();
  };

  let _ =
    Event.subscribe(
      nvimApi.onNotification,
      n => {
        let parsedEvent = Notification.parse(n.notificationType, n.payload);

        let f = n => {
          Event.dispatch(onNotification, n);
        };

        List.iter(f, parsedEvent);
      },
    );

  let ret: t = {uiAttach, input, onNotification, bufAttach};
  ret;
};
