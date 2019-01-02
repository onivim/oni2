/*
 * Oni2.re
 *
 * This is the entry point for launching the editor.
 */

open Revery;
open Revery.Core;
open Revery.UI;

open CamomileLibraryDefault.Camomile;
open Zed_utf8;

/* The 'main' function for our app */
let init = app => {
  let w =
    App.createWindow(
      ~createOptions={...Window.defaultCreateOptions, vsync: false, maximized: false},
      app,
      "Oni2",
    );

  let backgroundColor = Color.rgb(33. /. 255., 39. /. 255., 51. /. 255.);

  /* Set up some styles */
  let textHeaderStyle =
    Style.make(
      ~color=Colors.white,
      ~fontFamily="FiraCode-Regular.ttf",
      ~fontSize=14,
      (),
    );

  /* Set up some styles */
  let fontAwesomeStyle =
    Style.make(
      ~color=Colors.white,
      ~fontFamily="FontAwesome5FreeRegular.otf",
      ~fontSize=14,
      (),
    );


  let fontAwesomeIcon = Zed_utf8.singleton(UChar.of_int(0xF556));


  let render = () => {
    <view
        style={Style.make(
            ~backgroundColor=backgroundColor,
            ~position=LayoutTypes.Absolute,
            ~top=0,
            ~left=0,
            ~right=0,
            ~bottom=0,
            ~justifyContent=LayoutTypes.JustifyCenter,
            ~alignItems=LayoutTypes.AlignCenter,
            (),
        )}>
        <text style=fontAwesomeStyle>{fontAwesomeIcon}</text>
        <text style=textHeaderStyle>"Hello, World!"</text>
        <text style=fontAwesomeStyle>{fontAwesomeIcon}</text>
    </view>;
  };

  UI.start(w, render);
};

/* Let's get this party started! */
App.start(init);
