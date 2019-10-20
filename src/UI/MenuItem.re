open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Oni_Core.Types;
open Oni_Model;

let component = React.component("MenuItem");

module Constants = {
  let fontSize = 20;
}

module Styles = {
  let bg = (~theme: Theme.t, ~selected) =>
    selected ? theme.editorMenuItemSelected : theme.editorMenuBackground

  let text = (~theme: Theme.t, ~font: UiFont.t, ~selected) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      color(theme.editorMenuForeground),
      backgroundColor(bg(~theme, ~selected))
    ];

  let container = (~theme, ~selected) =>
    Style.[
      padding(10),
      flexDirection(`Row),
      backgroundColor(bg(~theme, ~selected))
    ];

  let icon = fgColor =>
    Style.[
      fontFamily("seti.ttf"),
      fontSize(Constants.fontSize),
      marginRight(10),
      color(fgColor),
    ];

  let label = (~font: UiFont.t, ~theme: Theme.t, ~selected, ~custom) =>
    Style.(
      merge(
        ~source=
          Style.[
            fontFamily(font.fontFile),
            textOverflow(`Ellipsis),
            fontSize(12),
            color(theme.editorMenuForeground),
            backgroundColor(bg(~theme, ~selected))
          ],
        ~target=custom,
      )
    );

  let clickable =
    Style.[
      cursor(Revery.MouseCursors.pointer)
    ];
}

let noop = () => ();

let createElement =
    (
      ~children as _,
      ~style=[],
      ~icon=None,
      ~label,
      ~selected,
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
        open IconTheme.IconDefinition;
        <Text
          style=Styles.icon(v.fontColor)
          text=FontIcon.codeToIcon(v.fontCharacter)
        />

      | None =>
        <Text style=Styles.icon(Colors.transparentWhite) text="" />
      };

    let labelView = 
      switch (label) {
        | `Text(text) =>
          let style = Styles.label(~font, ~theme, ~selected, ~custom=style);
          <Text style text />
        | `Custom(view) =>
          view
      };

    (
      hooks,
      <Clickable style=Styles.clickable onClick>
        <View onMouseOver={_ => onMouseOver()} style=Styles.container(~theme, ~selected)>
          iconView
          labelView
        </View>
      </Clickable>,
    );
  });
