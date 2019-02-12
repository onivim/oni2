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
    prerr_endline(
      "[DEBUG - STATE] Mode: "
      ++ Core.Types.Mode.show(state.mode)
      ++ " editor font measured width: "
      ++ string_of_int(state.editorFont.measuredWidth)
      ++ " editor font measured height: "
      ++ string_of_int(state.editorFont.measuredHeight),
    );
    <Root state />;
  };

  UI.start(w, render);

  let initVimPath =
    Revery_Core.Environment.getExecutingDirectory() ++ "init.vim";
  Core.Log.debug("initVimPath: " ++ initVimPath);

  let nvim =
    NeovimProcess.start(~neovimPath, ~args=[|"-u", initVimPath, "--embed"|]);
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

  let setFont = (fontFamily, fontSize) => {
    Fontkit.fk_new_face(
      Revery.Core.Environment.getExecutingDirectory() ++ fontFamily,
      fontSize,
      font => {
        open Oni_Core.Actions;
        open Oni_Core.Types;

        /* Measure text */
        let shapedText = Fontkit.fk_shape(font, "H");
        let firstShape = shapedText[0];
        let glyph = Fontkit.renderGlyph(font, firstShape.glyphId);

        let metrics = Fontkit.fk_get_metrics(font);

        let actualHeight = int_of_float(float_of_int(fontSize) *. float_of_int(metrics.height) /. float_of_int(metrics.unitsPerEm));

        /* Set editor text based on measurements */
        App.dispatch(
          app,
          SetEditorFont(
            EditorFont.create(
              ~fontFile=fontFamily,
              ~fontSize,
              ~measuredWidth=glyph.advance / 64,
              ~measuredHeight=actualHeight,
              (),
            ),
          ),
        );
      },
      _ => prerr_endline("setFont: Failed to load font " ++ fontFamily),
    );
  };

  setFont("FiraCode-Regular.ttf", 14);

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
    Event.subscribe(
      neovimProtocol.onNotification,
      n => {
        let msg =
          switch (n) {
          | ModeChanged("normal") => Core.Actions.ChangeMode(Normal)
          | ModeChanged("insert") => Core.Actions.ChangeMode(Insert)
          | ModeChanged(_) => Core.Actions.ChangeMode(Other)
          | CursorMoved(c) => Core.Actions.CursorMove(Core.Types.BufferPosition.create(
                c.cursorLine,
                c.cursorColumn,
          ))
          | BufferLines(bc) =>
            Core.Actions.BufferUpdate(
              Core.Types.BufferUpdate.create(
                ~startLine=bc.firstLine,
                ~endLine=bc.lastLine,
                ~lines=bc.lines,
                (),
              ),
            )
          | _ => Noop
          };

        App.dispatch(app, msg);
        prerr_endline("Protocol Notification: " ++ Notification.show(n));
      },
    );
  ();
};

/* Let's get this party started! */
App.startWithState(Core.State.create(), Core.Reducer.reduce, init);
