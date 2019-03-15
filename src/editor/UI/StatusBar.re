/*
 * StatusBar.re
 *
 * Container for StatusBar
 */

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

module StatusBarSection = {
    let component = React.component("StatusBarSection");

    let style = Style.[
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
        /* paddingHorizontal(5), */
    ];

    let createElement = (~children, ()) => component(hooks => {
        (hooks, 
        <View style>
            ...children
        </View>
        );
    });
    
}

module StatusBarItem = {
    let component = React.component("StatusBarItem");

    let style = Style.[
        flexDirection(`Column),
        justifyContent(`Center),
        alignItems(`Center),
        width(75),
        /* paddingHorizontal(5), */
    ];

    let createElement = (~children, ()) => component(hooks => {
        (hooks, 
        <View style>
            ...children
        </View>
        );
    });
}

let modeStyle = (mode, theme: Theme.t) => {
  open Types.Mode;
  open Theme;
  let (background, foreground) =
    switch (mode) {
    | Visual => (
        theme.colors.oniVisualModeBackground,
        theme.colors.oniVisualModeForeground,
      )
    | Commandline => (
        theme.colors.oniCommandlineModeBackground,
        theme.colors.oniCommandlineModeForeground,
      )
    | Operator => (
        theme.colors.oniOperatorModeBackground,
        theme.colors.oniOperatorModeForeground,
      )
    | Insert => (
        theme.colors.oniInsertModeBackground,
        theme.colors.oniInsertModeForeground,
      )
    | Replace => (
        theme.colors.oniReplaceModeBackground,
        theme.colors.oniReplaceModeForeground,
      )
    | Other
    | Normal => (
        theme.colors.oniNormalModeBackground,
        theme.colors.oniNormalModeForeground,
      )
    };
  Style.[
    backgroundColor(background),
    color(foreground),
  ];
};

let createElement =
    (
      ~children as _,
      ~state: State.t,
      (),
    ) =>
  component(hooks => {
    let mode = state.mode;
    let theme = state.theme;
    let position = state.editor.cursorPosition;

    let textStyle = getTextStyle(state.uiFont);

    (
      hooks,
      <View style=viewStyle>
        <StatusBarSection />
        <StatusBarSection />

        <StatusBarSection>
            <StatusBarItem>
                  <Text
                    style=Style.[
                      backgroundColor(theme.colors.background),
                      color(theme.colors.statusBarForeground),
                      ...textStyle,
                    ]
                    text={convertPositionToString(position)}
                  />
            </StatusBarItem>
            <StatusBarItem>
                <View style={modeStyle(mode, theme)}>
                  <Text style=textStyle text={Types.Mode.show(mode)} />
                </View>
            </StatusBarItem>
        </StatusBarSection>
      </View>,
    )}
  );
