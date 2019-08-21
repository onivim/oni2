open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Oni_Core.Types;
open Oni_Model;

let component = React.component("MenuItem");

let menuItemFontSize = 20;

let textStyles = (~theme: Theme.t, ~uiFont: UiFont.t, ~bg: Color.t, ()) =>
  Style.[
    fontFamily(uiFont.fontFile),
    fontSize(uiFont.fontSize),
    color(theme.editorMenuForeground),
    backgroundColor(bg),
  ];

let containerStyles = (~bg, ()) =>
  Style.[padding(10), flexDirection(`Row), backgroundColor(bg)];

let iconStyles = fgColor =>
  Style.[
    fontFamily("seti.ttf"),
    fontSize(menuItemFontSize),
    marginRight(10),
    color(fgColor),
  ];

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
        selected
          ? theme.editorMenuItemSelected
          : theme.editorMenuBackground
      );

    let labelStyles =
      Style.(
        merge(
          ~source=
            Style.[
              fontFamily(uiFont.fontFile),
              textOverflow(`Ellipsis),
              fontSize(uiFont.fontSize),
              color(theme.editorMenuForeground),
            ],
          ~target=style,
        )
      );

    let iconView =
      switch (icon) {
      | Some(v) =>
        IconTheme.IconDefinition.(
          <Text
            style={iconStyles(v.fontColor)}
            text={FontIcon.codeToIcon(v.fontCharacter)}
          />
        )
      | None => <Text style={iconStyles(Colors.transparentWhite)} text="" />
      };

    (
      hooks,
      <Clickable style=Style.[cursor(Revery.MouseCursors.pointer)] onClick>
        <View
          onMouseOver={_ => onMouseOver()} style={containerStyles(~bg, ())}>
          iconView
          <Text style=labelStyles text=label />
        </View>
      </Clickable>,
    );
  });
