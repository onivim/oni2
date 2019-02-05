/*
 * Oni2.re
 *
 * This is the entry point for launching the editor.
 */

open Revery;
open Revery.Core;
open Revery.UI;

open Rench;

open Oni_UI;
open Oni_Neovim;

module Core = Oni_Core;

exception NeovimNotFound;

/* The 'main' function for our app */
let init = app => {
  let w =
    App.createWindow(
      ~createOptions={
        ...Window.defaultCreateOptions,
        vsync: false,
        maximized: false,
      },
      app,
      "Oni2",
    );

  let neovimPath =
    switch (Environment.getEnvironmentVariable("ONI2_NEOVIM_PATH")) {
    | Some(p) => p
    | None => raise(NeovimNotFound)
    };

  let render = () => {
      let state: Core.State.t = App.getState(app);
      prerr_endline ("[STATE] Mode: " ++ Core.State.Mode.show(state.mode));
    <Root />;
  };

  UI.start(w, render);

  let nvim = NeovimProcess.start(~neovimPath, ~args=[|"--embed"|]);
  let msgpackTransport =
    MsgpackTransport.make(
      ~onData=nvim.stdout.onData,
      ~write=nvim.stdin.write,
      (),
    );

  let nvimApi: NeovimApi.t = NeovimApi.make(msgpackTransport);
  let neovimProtocol: NeovimProtocol.t = NeovimProtocol.make(nvimApi);

  neovimProtocol.uiAttach();

  /* let buf = nvimApi.requestSync( */
  /*   "nvim_get_current_buf", */
  /*   Msgpck.List([]), */
  /* ); */
  /* prerr_endline ("BUF: " ++ Msgpck.show(buf)); */

  let _ =
    nvimApi.requestSync(
      "nvim_buf_attach",
      Msgpck.List([Msgpck.Int(0), Msgpck.Bool(true), Msgpck.Map([])]),
    );

  let _ =
    Event.subscribe(
      w.onKeyPress,
      event => {
        let c = event.character;
        neovimProtocol.input(c);
      },
    );

  let _ =
    Event.subscribe(
      w.onKeyDown,
      event => {
        let _ =
          switch (event.key) {
          | Key.KEY_BACKSPACE => ignore(neovimProtocol.input("<BS>"))
          | Key.KEY_ENTER => ignore(neovimProtocol.input("<CR>"))
          | Key.KEY_ESCAPE => ignore(neovimProtocol.input("<ESC>"))
          | _ => ()
          };
        ();
      },
    );

  let _ = Tick.interval(_ => nvimApi.pump(), Seconds(0.));

  let _ =
    Event.subscribe(nvimApi.onNotification, n =>
      prerr_endline(
        "Raw Notification: "
        ++ n.notificationType
        ++ " | "
        ++ Msgpck.show(n.payload),
      )
    );

  let _ =
    Event.subscribe(neovimProtocol.onNotification, n => {
      let msg = switch (n) {
      | ModeChanged("normal") => Core.Actions.ChangeMode(Normal)
      | ModeChanged("insert") => Core.Actions.ChangeMode(Insert)
      | ModeChanged(_) => Core.Actions.ChangeMode(Other)
      | _ => Noop
      };

      App.dispatch(app, msg);
      prerr_endline("Protocol Notification: " ++ Notification.show(n))
    });
  ();
};


/* Let's get this party started! */
App.startWithState(Core.State.create(), Core.Reducer.reduce, init);
