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
  Style.make(~fontFamily="FiraCode-Regular.ttf", ~fontSize=14, ());

/* Set up some styles */
let fontAwesomeStyle =
  Style.make(~fontFamily="FontAwesome5FreeRegular.otf", ~fontSize=14, ());

let fontAwesomeIcon = Zed_utf8.singleton(UChar.of_int(0xF556));

let _viewLinesToElements = (_bufferView: array(BufferViewLine.t)) => {
  let ret = [
    <text style=textHeaderStyle> "Hello" </text>,
    <text style=textHeaderStyle> "World" </text>,
  ];
  ret;
};

include (
          val component((render, ~children, ()) =>
                render(
                  () => {
                    let theme = useContext(Theme.context);

                    let bufferView =
                      Buffer.ofLines([|
                        "- Hello from line 1",
                        "- Hello from line 2",
                        "--- Hello from line 3",
                      |])
                      |> TokenizedBuffer.ofBuffer
                      |> TokenizedBufferView.ofTokenizedBuffer;

                    let textElements =
                      _viewLinesToElements(bufferView.viewLines);

                    <view
                      style={Style.make(
                        /* ~backgroundColor=Colors.red, */
                        ~backgroundColor=theme.background,
                        ~color=theme.foreground,
                        ~flexGrow=1,
                        ~flexShrink=1,
                        (),
                      )}>
                      ...textElements
                    </view>;
                  },
                  ~children,
                )
              )
        );
