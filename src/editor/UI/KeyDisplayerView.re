/*
 * KeyDisplayerView.re
 *
 * View for KeyDisplayer
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
open Oni_Model;

open Oni_Model.StatusBarModel;

let bgc = Color.rgb(0.1, 0.1, 0.1);
let fgc = Color.rgb(0.9, 0.9, 0.9);

let containerStyle =
  Style.[
    backgroundColor(bgc),
    justifyContent(`Center),
    alignItems(`Center),
    margin(8),
    flexGrow(0),
  ];

let textStyle = uiFont => {
  Style.[
    fontFamily(uiFont),
    fontSize(24),
    textWrap(TextWrapping.NoWrap),
    backgroundColor(bgc),
    color(fgc),
    marginHorizontal(16),
    marginVertical(8),
    flexGrow(0),
  ];
};

let createElement = (~children as _, ~state: State.t, ()) => {
  let uiFont = state.uiFont.fontFile;

  <Positioned bottom=50 right=50>
    <View
      style=Style.[
        backgroundColor(bgc),
        justifyContent(`Center),
        alignItems(`Center),
        margin(8),
        flexGrow(0),
      ]>
      <Text
        style=Style.[
          fontFamily(uiFont),
          fontSize(24),
          textWrap(TextWrapping.NoWrap),
          backgroundColor(bgc),
          color(Colors.white),
          marginHorizontal(16),
          marginVertical(8),
          flexGrow(0),
        ]
        text="a"
      />
    </View>
    <View
      style=Style.[
        backgroundColor(bgc),
        justifyContent(`Center),
        alignItems(`Center),
        margin(8),
        flexGrow(0),
      ]>
      <Text
        style=Style.[
          fontFamily(uiFont),
          fontSize(24),
          textWrap(TextWrapping.NoWrap),
          backgroundColor(bgc),
          color(Colors.white),
          marginHorizontal(16),
          marginVertical(8),
          flexGrow(0),
        ]
        text="Ctrl+a"
      />
    </View>
  </Positioned>;
};
