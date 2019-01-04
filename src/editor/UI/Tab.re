/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery.UI;
open Revery.UI.Components;

type tabAction = unit => unit;

let tabHeight = 35;
let maxWidth = 145;
let fontFamily = "Inter-UI-SemiBold.ttf";
let fontSize = 12;

include (
          val component((render, ~title, ~active:bool, ~onClick:tabAction, ~onClose:tabAction, ~children, ()) =>
                render(
                  () => {
                    let _ = (onClick, onClose);
                    let theme = useContext(Theme.context);

                    let opacity = active ? 1.0 : 0.6;

                    <Clickable>
                        <view
                          style={Style.make(
                            ~backgroundColor=theme.editorBackground,
                            ~opacity,
                            ~height=tabHeight,
                            ~width=maxWidth,
                            ~flexDirection=LayoutTypes.Row,
                            ~justifyContent=LayoutTypes.JustifyCenter,
                            ~alignItems=LayoutTypes.AlignCenter,
                            (),
                          )}>
                            <text style={
                                Style.make(
                                    ~fontFamily,
                                    ~fontSize,
                                    ~color=theme.editorForeground,
                                    ()
                                )
                            }>{title}</text>
                        </view>
                    </Clickable>;
                  },
                  ~children,
                )
              )
        );
