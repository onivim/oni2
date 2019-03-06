/*
 * StatusBar.re
 *
 * Container for StatusBar
 */

open Revery.UI;

open Oni_Core;

let component = React.component("StatusBar");

let textStyle =
  Style.[
    fontFamily("Inter-UI-Regular.ttf"),
    fontSize(14),
    paddingHorizontal(5),
  ];

let viewStyle =
  Style.[
    flexDirection(`Row),
    justifyContent(`SpaceBetween),
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

let modeStyle = (mode, theme: Theme.t) => {
  open Types.Mode;
  open Theme;
  let (background, foreground) =
    switch (mode) {
    | Visual => (
        theme.colors.statusBarItemVisualModeBackground,
        theme.colors.statusBarItemVisualModeForeground,
      )
    | Commandline => (
        theme.colors.statusBarItemCommandlineModeBackground,
        theme.colors.statusBarItemCommandlineModeForeground,
      )
    | Operator => (
        theme.colors.statusBarItemOperatorModeBackground,
        theme.colors.statusBarItemOperatorModeForeground,
      )
    | Insert => (
        theme.colors.statusBarItemInsertModeBackground,
        theme.colors.statusBarItemInsertModeForeground,
      )
    | Replace => (
        theme.colors.statusBarItemReplaceModeBackground,
        theme.colors.statusBarItemReplaceModeForeground,
      )
    | Other
    | Normal => (
        theme.colors.statusBarItemNormalModeBackground,
        theme.colors.statusBarItemNormalModeForeground,
      )
    };
  Style.[
    backgroundColor(background),
    color(foreground),
    alignSelf(`FlexStart),
    paddingHorizontal(5),
  ];
};

let createElement =
    (
      ~children as _,
      ~mode: Types.Mode.t,
      ~theme: Theme.t,
      ~position: Types.BufferPosition.t,
      (),
    ) =>
  component(hooks =>
    (
      hooks,
      <View style=viewStyle>
        <View style={modeStyle(mode, theme)}>
          <Text style=textStyle text={Types.Mode.show(mode)} />
        </View>
        <View
          style=Style.[
            backgroundColor(theme.colors.statusBarItemBackground),
            paddingHorizontal(5),
          ]>
          <Text
            style=Style.[
              color(theme.colors.statusBarItemForeground),
              ...textStyle,
            ]
            text={convertPositionToString(position)}
          />
        </View>
      </View>,
    )
  );
