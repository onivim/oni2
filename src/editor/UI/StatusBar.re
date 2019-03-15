/*
 * StatusBar.re
 *
 * Container for StatusBar
 */

open Revery;
open Revery.UI;

open Oni_Core;
open Oni_Model;

let component = React.component("StatusBar");

let getTextStyle = (uiFont) => {
  open Oni_Core.Types.UiFont;

  Style.[
    fontFamily(uiFont.fontFile),
    fontSize(11),
  ];
};

let viewStyle =
  Style.[
    flexDirection(`Row),
    alignItems(`Center),
    flexGrow(1),
    justifyContent(`FlexEnd),
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
  ];

let convertPositionToString = (position: Types.BufferPosition.t) =>
  string_of_int(Types.Index.toOneBasedInt(position.line))
  ++ ","
  ++ string_of_int(Types.Index.toOneBasedInt(position.character));

module StatusBarItem = {
    let component = React.component("StatusBarItem");

    let getStyle = (h, bg: Color.t) => Style.[
        flexDirection(`Column),
        justifyContent(`Center),
        alignItems(`Center),
        height(h),
        width(75),
        backgroundColor(bg),
        paddingHorizontal(5),
    ];

    let createElement = (~children, ~height, ~backgroundColor, ()) => component(hooks => {
        
        (hooks, 
        <View style=getStyle(height, backgroundColor)>
            ...children
        </View>
        );
    });
}

let createElement =
    (
      ~children as _,
      ~height,
      ~state: State.t,
      (),
    ) =>
  component(hooks => {
    let mode = state.mode;
    let theme = state.theme;
    let position = state.editor.cursorPosition;

    let textStyle = getTextStyle(state.uiFont);

  let (background, foreground) = Theme.getColorsForMode(theme, mode);

    (
      hooks,
      <View style=viewStyle>
            <StatusBarItem height backgroundColor={theme.colors.background}>
                  <Text
                    style=Style.[
                      backgroundColor(theme.colors.background),
                      color(theme.colors.statusBarForeground),
                      ...textStyle,
                    ]
                    text={convertPositionToString(position)}
                  />
            </StatusBarItem>
            <StatusBarItem height backgroundColor=background>
                  <Text style=Style.[
                    backgroundColor(background),
                    color(foreground),
                    ...textStyle,
                  ] text={Types.Mode.show(mode)} />
            </StatusBarItem>
      </View>,
    )}
  );
