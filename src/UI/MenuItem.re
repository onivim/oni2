open Revery;
open Revery.UI;
open Oni_Core;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;

module Colors = Feature_Theme.Colors;

module Constants = {
  let iconSize = 20.;
};

module Styles = {
  let bg = (~isFocused) =>
    isFocused ? Colors.List.focusBackground : Colors.Menu.background;

  let text = (~theme, ~font: UiFont.t, ~isFocused) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      color(Colors.Menu.foreground.from(theme)),
      backgroundColor(bg(~isFocused).from(theme)),
    ];

  let container = (~theme, ~isFocused) =>
    Style.[
      padding(10),
      flexDirection(`Row),
      backgroundColor(bg(~isFocused).from(theme)),
    ];

  let icon = fg =>
    Style.[
      fontFamily("seti.ttf"),
      fontSize(Constants.iconSize),
      color(fg),
      width(int_of_float(Constants.iconSize *. 0.75)),
      height(int_of_float(Constants.iconSize *. 0.85)),
      textWrap(TextWrapping.NoWrap),
      marginRight(10),
    ];

  let label = (~font: UiFont.t, ~theme, ~isFocused, ~custom) =>
    Style.(
      merge(
        ~source=
          Style.[
            fontFamily(font.fontFile),
            textOverflow(`Ellipsis),
            fontSize(12.),
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
      ~style=[],
      ~icon=None,
      ~font,
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
          text={FontIcon.codeToIcon(v.fontCharacter)}
        />
      )

    | None =>
      <Text style={Styles.icon(Revery.Colors.transparentWhite)} text="" />
    };

  let labelView =
    switch (label) {
    | `Text(text) =>
      let style = Styles.label(~font, ~theme, ~isFocused, ~custom=style);
      <Text style text />;
    | `Custom(view) => view
    };

  <Sneakable style=Styles.clickable onClick>
    <View
      onMouseOver={_ => onMouseOver()}
      style={Styles.container(~theme, ~isFocused)}>
      iconView
      labelView
    </View>
  </Sneakable>;
};
