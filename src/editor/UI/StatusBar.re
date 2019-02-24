/*
 * StatusBar.re
 *
 * Container for StatusBar
 */

open Revery.Core;
open Revery.UI;

open Oni_Core;

let component = React.component("StatusBar");

let textStyle =
  Style.[
    color(Color.hex("#9da5b4")),
    fontFamily("Inter-UI-Regular.ttf"),
    fontSize(14),
    paddingHorizontal(5),
  ];

let viewStyle =
  Style.[
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
  ];

let convertPositionToString = (position: Types.BufferPosition.t) => {
  string_of_int(Types.Index.toOneBasedInt(position.line))
  ++ ","
  ++ string_of_int(Types.Index.toOneBasedInt(position.character));
};

let createElement =
    (
      ~children as _,
      ~mode: Types.Mode.t,
      ~position: Types.BufferPosition.t,
      (),
    ) =>
  component(hooks =>
    (
      hooks,
      <View>
        <View style=viewStyle>
          <Text style=textStyle text={Types.Mode.show(mode)} />
          <Text style=textStyle text={convertPositionToString(position)} />
        </View>
      </View>,
    )
  );
