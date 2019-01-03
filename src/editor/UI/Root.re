/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery.UI;

open CamomileLibraryDefault.Camomile;

/* Set up some styles */
let textHeaderStyle =
  Style.make(~fontFamily="FiraCode-Regular.ttf", ~fontSize=14, ());

/* Set up some styles */
let fontAwesomeStyle =
  Style.make(~fontFamily="FontAwesome5FreeRegular.otf", ~fontSize=14, ());

let fontAwesomeIcon = Zed_utf8.singleton(UChar.of_int(0xF556));

include (
          val component((render, ~children, ()) =>
                render(
                  () => {
                    let theme = useContext(Theme.context);

                    <view
                      style={Style.make(
                        ~backgroundColor=theme.background,
                        ~color=theme.foreground,
                        ~position=LayoutTypes.Absolute,
                        ~top=0,
                        ~left=0,
                        ~right=0,
                        ~bottom=0,
                        ~justifyContent=LayoutTypes.JustifyCenter,
                        ~alignItems=LayoutTypes.AlignCenter,
                        (),
                      )}>
                      <text style=fontAwesomeStyle> fontAwesomeIcon </text>
                      <text style=textHeaderStyle> "Hello, World!" </text>
                      <text style=fontAwesomeStyle> fontAwesomeIcon </text>
                    </view>;
                  },
                  ~children,
                )
              )
        );
