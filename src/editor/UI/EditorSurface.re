/*
 * EditorSurface.re
 *
 * Component that handles rendering of the 'editor surface' -
 * the view of the buffer in the window.
 */

/* open Revery.Core; */
open Revery.UI;

open CamomileLibraryDefault.Camomile;

open Oni_Core;
open Oni_Core.TokenizedBufferView;

/* Set up some styles */
let textHeaderStyle =
  Style.[fontFamily("FiraCode-Regular.ttf"), fontSize(14)];

/* Set up some styles */
let fontAwesomeStyle =
  Style.[fontFamily("FontAwesome5FreeRegular.otf"), fontSize(14)];

let fontAwesomeIcon = Zed_utf8.singleton(UChar.of_int(0xF556));

let _viewLinesToElements = (_bufferView: array(BufferViewLine.t)) => {
  let ret = [
    <Text style=textHeaderStyle text="Hello" />,
    <Text style=textHeaderStyle text="World" />,
  ];
  ret;
};

let component = React.component("EditorSurface");

let make = () =>
  component((_slots: React.Hooks.empty) => {
    let theme = Theme.get();

    let bufferView =
      Buffer.ofLines([|
        "- Hello from line 1",
        "- Hello from line 2",
        "--- Hello from line 3",
      |])
      |> TokenizedBuffer.ofBuffer
      |> TokenizedBufferView.ofTokenizedBuffer;

    let textElements = _viewLinesToElements(bufferView.viewLines);

    let style =
      Style.
        [
          backgroundColor(theme.background),
          color(theme.foreground),
          flexGrow(1),
        ];
        /* flexShrink(1), */

    <View style> ...textElements </View>;
  });

let createElement = (~children as _, ()) => React.element(make());
