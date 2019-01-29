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
let fontName = "Inter-UI-SemiBold.ttf";
let fontPixelSize = 12;

let component = React.component("Tab");

let make = (~title, ~active, ~onClick, ~onClose, ()) => component((_slots: React.Hooks.empty) => {
    

                  /* ~title, */
                  /* ~active: bool, */
                  /* ~onClick: tabAction, */
                  /* ~onClose: tabAction, */
                  /* ~children, */
                    let _ = (onClick, onClose);
                    let theme = Theme.get();

                    let opacityValue = active ? 1.0 : 0.6;

                    let containerStyle = Style.[
                        backgroundColor(theme.editorBackground),
                        opacity(opacityValue),
                        height(tabHeight),
                        width(maxWidth),
                        flexDirection(`Row),
                        justifyContent(`Center),
                        alignItems(`Center),
                    ];

                    let textStyle = Style.[
                        fontFamily(fontName),
                        fontSize(fontPixelSize),
                        color(theme.editorForeground),
                    ];

                    <Clickable>
                      <View
                        style={containerStyle}>
                        <Text
                          style={textStyle} textContent={title} />
                      </View>
                    </Clickable>;
        });

let createElement = (~title, ~active, ~onClick, ~onClose, ~children as _, ()) => {
   React.element(make(~title, ~active, ~onClick, ~onClose, ())); 
}
