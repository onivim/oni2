open Revery;
open Revery.UI;
open Oni_Core;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;
module Sneakable = Feature_Sneak.View.Sneakable;

module Colors = Feature_Theme.Colors;

module View = Revery.UI.View;

module Constants = {
  let iconSize = 20.;
};

module Styles = {
  let bg = (~isFocused) =>
    isFocused ? Colors.List.focusBackground : Colors.Menu.background;

  let container = (~theme, ~isFocused) =>
    Style.[
      padding(10),
      flexDirection(`Row),
      backgroundColor(bg(~isFocused).from(theme)),
    ];

  let icon = fg =>
    Style.[
      color(fg),
      width(int_of_float(Constants.iconSize *. 0.75)),
      height(int_of_float(Constants.iconSize *. 0.85)),
      textWrap(TextWrapping.NoWrap),
      marginRight(10),
    ];

  let label = (~theme, ~isFocused, ~custom) =>
    Style.(
      merge(
        ~source=
          Style.[
            textOverflow(`Ellipsis),
            color(Colors.Menu.foreground.from(theme)),
            backgroundColor(bg(~isFocused).from(theme)),
          ],
        ~target=custom,
      )
    );

  let clickable = Style.[cursor(Revery.MouseCursors.pointer)];
};

let noop = () => ();

let make =
    (
      ~icon=None,
      ~label,
      ~isFocused,
      ~theme,
      ~onClick=noop,
      ~onMouseOver=noop,
      (),
    ) => {
  let iconView =
    switch (icon) {
    | Some(v) =>
      IconTheme.IconDefinition.(
        <Text
          style={Styles.icon(v.fontColor)}
          fontFamily={Revery.Font.Family.fromFile("seti.ttf")}
          fontSize=Constants.iconSize
          text={FontIcon.codeToIcon(v.fontCharacter)}
        />
      )

    | None =>
      <Text style={Styles.icon(Revery.Colors.transparentWhite)} text="" />
    };

  let labelView =
    switch (label) {
    // | `Text(text) =>
    // <Text
    //   style={Styles.label(~theme, ~isFocused, ~custom=style)}
    //   fontFamily={font.family}
    //   fontSize
    //   text
    // />;
    | `Custom(view) => view
    };

  <Sneakable sneakId="menuItem" style=Styles.clickable onClick>
    <View
      onMouseOver={_ => onMouseOver()}
      style={Styles.container(~theme, ~isFocused)}>
      iconView
      labelView
    </View>
  </Sneakable>;
};
