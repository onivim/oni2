open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Core.Types;
open Oni_Model;

let component = React.component("MenuItem");

let menuItemFontSize = 20;

let textStyles = (~theme: Theme.t, ~uiFont: UiFont.t, ~bg: Color.t, ()) =>
  Style.[
    fontFamily(uiFont.fontFile),
    fontSize(uiFont.fontSize),
    color(theme.colors.editorMenuForeground),
    backgroundColor(bg),
  ];

let containerStyles = (~bg, ()) =>
  Style.[padding(10), flexDirection(`Row), backgroundColor(bg)];

let iconStyles =
  Style.[
    fontFamily("FontAwesome5FreeSolid.otf"),
    fontSize(menuItemFontSize),
    marginRight(10),
  ];

let createElement =
    (~children as _, ~style=[], ~icon={||}, ~label, ~selected, ~theme, ()) =>
  component(hooks => {
    let state = GlobalContext.current().state;
    let uiFont = State.(state.uiFont);

    let bg: Color.t =
      Theme.(
        selected ?
          theme.colors.editorMenuItemSelected :
          theme.colors.editorMenuBackground
      );

    let labelStyles =
      Style.(
        merge(
          ~source=
            Style.[
              fontFamily(uiFont.fontFile),
              textOverflow(`Ellipsis),
              fontSize(uiFont.fontSize),
              color(theme.colors.editorMenuForeground),
              backgroundColor(bg),
            ],
          ~target=style,
        )
      );
    (
      hooks,
      <View style={containerStyles(~bg, ())}>
        <Text style=iconStyles text=icon />
        <Text style=labelStyles text=label />
      </View>,
    );
  });
