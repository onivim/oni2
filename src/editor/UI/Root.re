/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery.UI;

include (
          val component((render, ~children, ()) =>
                render(
                  () => {
                    let theme = Theme.get();

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
                      <Editor />
                    </view>;
                  },
                  ~children,
                )
              )
        );
