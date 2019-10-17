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
  let text = (~theme: Theme.t, ~uiFont: UiFont.t, ~bg: Color.t) =>
    Style.[
      fontFamily(uiFont.fontFile),
      fontSize(uiFont.fontSize),
      color(theme.editorMenuForeground),
      backgroundColor(bg),
    ];

  let container = (~bg) =>
    Style.[padding(10), flexDirection(`Row), backgroundColor(bg)];

  let icon = fgColor =>
    Style.[
      fontFamily("seti.ttf"),
      fontSize(Constants.fontSize),
      marginRight(10),
      color(fgColor),
    ];

  let label = (~font, ~fg, ~bg, ~custom) =>
    Style.(
      merge(
        ~source=
          Style.[
            fontFamily(font),
            textOverflow(`Ellipsis),
            fontSize(12),
            color(fg),
            backgroundColor(bg),
          ],
        ~target=custom,
      )
    );
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
    let uiFont = State.(state.uiFont);

    let bg: Color.t =
      Theme.(
        selected ? theme.editorMenuItemSelected : theme.editorMenuBackground
      );


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
          let style = Styles.label(~font=uiFont.fontFile, ~fg=theme.editorMenuForeground, ~bg, ~custom=style);
          <Text style text />
        | `Custom(view) =>
          view
      };

    (
      hooks,
      <Clickable style=Style.[cursor(Revery.MouseCursors.pointer)] onClick>
        <View onMouseOver={_ => onMouseOver()} style=Styles.container(~bg)>
          iconView
          labelView
        </View>
      </Clickable>,
    );
  });
