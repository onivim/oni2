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

  let neovimPath = switch(Environment.getEnvironmentVariable("ONI2_NEOVIM_PATH")) {
  | Some(p) => p
  | None => raise(NeovimNotFound);
  }


  let render = () => {
      <Root />
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
  let _ = nvimApi.requestSync(
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

  let _ = Event.subscribe(w.onKeyPress, (event) => {
      let c = event.character;
    let _ = nvimApi.requestSync("nvim_input", Msgpck.List([Msgpck.String(c)]));
  });

  let _ = Tick.interval((_) => {
    nvimApi.pump(); 
  }, Seconds(0.));

  let _  = Event.subscribe(nvimApi.onNotification, (n) => {
    prerr_endline ("Notification: " ++ n.notificationType ++ " | " ++ Msgpck.show(n.payload)); 
  });
};

/* Let's get this party started! */
App.start(init);
