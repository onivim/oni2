open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;

let component = React.component("Menu");

let menuWidth = 400;
let menuHeight = 350;

let containerStyles = (theme: Theme.t) =>
  Style.[
    backgroundColor(theme.colors.editorMenuBackground),
    color(theme.colors.editorMenuForeground),
    width(menuWidth),
    height(menuHeight),
    boxShadow(
      ~xOffset=-15.,
      ~yOffset=5.,
      ~blurRadius=30.,
      ~spreadRadius=5.,
      ~color=Color.rgba(0., 0., 0., 0.2),
    ),
  ];

let paletteItemStyle = Style.[fontSize(14)];

let getIcon = icon =>
  switch (icon) {
  | Some(i) => i
  | None => ""
  };

let handleChange = (~value) =>
  GlobalContext.current().dispatch(MenuSearch(value));

let createElement =
    (
      ~children as _,
      ~font: Types.UiFont.t,
      ~menu: Types.UiMenu.t,
      ~theme: Theme.t,
      (),
    ) =>
  component(hooks =>
    (
      hooks,
      menu.isOpen ?
        <View style={containerStyles(theme)}>
          <Input
            placeholder="type here to search the menu"
            style=Style.[width(menuWidth), fontFamily(font.fontFile)]
            onChange=handleChange
          />
          <ScrollView style=Style.[height(menuHeight - 50)]>
            ...{
                 List.mapi(
                   (index, cmd: Types.UiMenu.command) =>
                     <MenuItem
                       icon={getIcon(cmd.icon)}
                       style=paletteItemStyle
                       label={cmd.name}
                       selected={index == menu.selectedItem}
                       theme
                     />,
                   menu.commands,
                 )
               }
          </ScrollView>
        </View> :
        React.listToElement([]),
    )
  );
