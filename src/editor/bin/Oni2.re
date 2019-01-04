/*
 * Oni2.re
 *
 * This is the entry point for launching the editor.
 */

open Revery;
open Revery.Core;
open Revery.UI;

open Oni_Core;
open Oni_UI;

hello();

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

  let themeProvider = Theme.provider;

  let render = () => {
      <themeProvider value={Theme.default}>
          <Root />
      </themeProvider>;
  };

  UI.start(w, render);
};

/* Let's get this party started! */
App.start(init);
