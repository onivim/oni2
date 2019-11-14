open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Oni_Core.Types;
open Oni_Model;

let component = React.component("MenuItem");

module Constants = {
  let fontSize = 20;
};

module Styles = {
  let bg = (~theme: Theme.t, ~isSelected) =>
    isSelected ? theme.menuSelectionBackground : theme.menuBackground;

  let text = (~theme: Theme.t, ~font: UiFont.t, ~isSelected) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      color(theme.menuForeground),
      backgroundColor(bg(~theme, ~isSelected)),
    ];

  let container = (~theme, ~isSelected) =>
    Style.[
      padding(10),
      flexDirection(`Row),
      backgroundColor(bg(~theme, ~isSelected)),
    ];

  let icon = fgColor =>
    Style.[
      fontFamily("seti.ttf"),
      fontSize(Constants.fontSize),
      marginRight(10),
      color(fgColor),
    ];

  let label = (~font: UiFont.t, ~theme: Theme.t, ~isSelected, ~custom) =>
    Style.(
      merge(
        ~source=
          Style.[
            fontFamily(font.fontFile),
            textOverflow(`Ellipsis),
            fontSize(12),
            color(theme.menuForeground),
            backgroundColor(bg(~theme, ~isSelected)),
          ],
        ~target=custom,
      )
    );

  let clickable = Style.[cursor(Revery.MouseCursors.pointer)];
};

let noop = () => ();

let createElement =
    (
      ~children as _,
      ~style=[],
      ~icon=None,
      ~label,
      ~isSelected,
      ~theme,
      ~onClick=noop,
      ~onMouseOver=noop,
      (),
    ) =>
  component(hooks => {
    let state = GlobalContext.current().state;
    let font = State.(state.uiFont);

    let iconView =
      switch (icon) {
      | Some(v) =>
        IconTheme.IconDefinition.(
          <Text
            style={Styles.icon(v.fontColor)}
            text={FontIcon.codeToIcon(v.fontCharacter)}
          />
        )

      | None => <Text style={Styles.icon(Colors.transparentWhite)} text="" />
      };

    let labelView =
      switch (label) {
      | `Text(text) =>
        let style = Styles.label(~font, ~theme, ~isSelected, ~custom=style);
        <Text style text />;
      | `Custom(view) => view
      };

    (
      hooks,
      <Clickable style=Styles.clickable onClick>
        <View
          onMouseOver={_ => onMouseOver()}
          style={Styles.container(~theme, ~isSelected)}>
          iconView
          labelView
        </View>
      </Clickable>,
    );
  });
