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
  let (background, foreground) =
      Theme.getColorsForMode(theme, mode);

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
        <View style=Style.[paddingHorizontal(5)]>
          <Text
            style=Style.[
              color(theme.colors.statusBarForeground),
              ...textStyle,
            ]
            text={convertPositionToString(position)}
          />
        </View>
      </View>,
    )
  );
