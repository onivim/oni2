/*
 * WelcomeView.re
 *
 * Component for the 'welcome' experience
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Model;
module Model = Oni_Model;

let editorViewStyle = (background, foreground) =>
  Style.[
    backgroundColor(background),
    color(foreground),
    flexGrow(1),
    flexDirection(`Column),
    justifyContent(`Center),
    alignItems(`Center)
  ];

let make = (~state: State.t, ()) => {
  let theme = state.theme;
  let style = editorViewStyle(theme.background, theme.foreground);

  <View style>
    <Image src="./title-logo.png" width=456 height=250/>
    <Text style=Style.[
      fontFamily(state.uiFont.fontFile),
      fontSize(20),
    ] text="Modal Editing from the Future" />
  </View>
};
