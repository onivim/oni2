open Oni_Core;

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Colors = Feature_Theme.Colors;

module Constants = {
  let menuWidth = 200;
  // let maxMenuHeight = 600;
};

// MODEL

[@deriving show({with_path: false})]
type item('data) = {
  label: string,
  // icon: option(IconTheme.IconDefinition.t),
  data: [@opaque] 'data,
};

// MENUITEM

module MenuItem = {
  module Constants = {
    let fontSize = 12.;
  };

  module Styles = {
    open Style;

    let bg = (~isFocused) =>
      isFocused ? Colors.Menu.selectionBackground : Colors.Menu.background;

    let container = (~theme, ~isFocused) => [
      padding(10),
      flexDirection(`Row),
      backgroundColor(bg(~isFocused).from(theme)),
    ];

    // let icon = fgColor => [
    //   fontFamily("seti.ttf"),
    //   fontSize(Constants.fontSize),
    //   marginRight(10),
    //   color(fgColor),
    // ];

    let label = (~theme, ~isFocused) => [
      textOverflow(`Ellipsis),
      color(Colors.Menu.foreground.from(theme)),
      backgroundColor(bg(~isFocused).from(theme)),
    ];
  };

  let component = React.Expert.component("MenuItem");
  let make:
    'data.
    (
      ~item: item('data),
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~onClick: unit => unit,
      unit
    ) =>
    _
   =
    (~item, ~theme, ~font, ~onClick, ()) =>
      component(hooks => {
        let ((isFocused, setIsFocused), hooks) = Hooks.state(false, hooks);

        // let iconView =
        //   switch (item.icon) {
        //   | Some(icon) =>
        //     IconTheme.IconDefinition.(
        //       <Text
        //         style={Styles.icon(icon.fontColor)}
        //         text={FontIcon.codeToIcon(icon.fontCharacter)}
        //       />
        //     )

        //   | None => <Text style={Styles.icon(Colors.transparentWhite)} text="" />
        //   };

        let labelView = {
          let style = Styles.label(~theme, ~isFocused);
          <Text
            fontFamily={font.family}
            fontSize=Constants.fontSize
            style
            text={item.label}
          />;
        };

        (
          <Clickable onClick>
            <View
              style={Styles.container(~theme, ~isFocused)}
              onMouseOut={_ => setIsFocused(_ => false)}
              onMouseOver={_ => setIsFocused(_ => true)}>
              // iconView
               labelView </View>
          </Clickable>,
          hooks,
        );
      });
};

// MENU

module Menu = {
  module Styles = {
    open Style;

    let container = (~x, ~y, ~theme) => [
      position(`Absolute),
      top(y),
      left(x),
      backgroundColor(Colors.Menu.background.from(theme)),
      color(Colors.Menu.foreground.from(theme)),
      width(Constants.menuWidth),
      boxShadow(
        ~xOffset=3.,
        ~yOffset=3.,
        ~blurRadius=5.,
        ~spreadRadius=0.,
        ~color=Color.rgba(0., 0., 0., 0.2),
      ),
    ];
  };

  let component = React.Expert.component("Menu");
  let make = (~items, ~x, ~y, ~orientation, ~theme, ~font, ~onItemSelect, ()) =>
    component(hooks => {
      let ((maybeRef, setRef), hooks) = Hooks.state(None, hooks);
      let (orientY, orientX) = orientation;

      let height =
        switch (maybeRef) {
        | Some((node: node)) => node#measurements().height
        | None => List.length(items) * 20
        };
      let width = Constants.menuWidth;

      let x =
        switch (orientX) {
        | `Left => x
        | `Middle => x - width / 2
        | `Right => x - width
        };

      let y =
        switch (orientY) {
        | `Top => y - height
        | `Middle => y - height / 2
        | `Bottom => y
        };

      (
        <View
          style={Styles.container(~x, ~y, ~theme)}
          ref={node => setRef(_ => Some(node))}>
          {items
           |> List.map(item => {
                let onClick = () => onItemSelect(item);
                <MenuItem item theme font onClick />;
              })
           |> React.listToElement}
        </View>,
        hooks,
      );
    });
};

// OVERLAY

module Overlay = {
  let internalSetMenus = ref(_ => ());

  let setMenu = (id, menu) => internalSetMenus^(IntMap.add(id, menu));
  let clearMenu = id => internalSetMenus^(IntMap.remove(id));

  module Styles = {
    open Style;

    let backdrop = [
      position(`Absolute),
      top(0),
      bottom(0),
      left(0),
      right(0),
      pointerEvents(`Allow),
      cursor(MouseCursors.arrow),
    ];
  };

  let%component make = (~onClick, ()) => {
    let%hook (menus, setMenus) = Hooks.state(IntMap.empty);
    internalSetMenus := setMenus;

    if (IntMap.is_empty(menus)) {
      React.empty;
    } else {
      <Clickable onClick style=Styles.backdrop>
        {IntMap.bindings(menus) |> List.map(snd) |> React.listToElement}
      </Clickable>;
    };
  };
};

// ANCHOR

module Anchor = {
  let generateId = {
    let lastId = ref(0);

    () => {
      incr(lastId);
      lastId^;
    };
  };

  let component = React.Expert.component("Anchor");
  let make =
      (
        ~items,
        ~orientation=(`Bottom, `Left),
        ~offsetX=0,
        ~offsetY=0,
        ~onItemSelect,
        ~theme,
        ~font,
        (),
      ) =>
    component(hooks => {
      let ((id, _), hooks) = Hooks.state(generateId(), hooks);
      let ((maybeRef, setRef), hooks) = Hooks.state(None, hooks);
      let ((), hooks) =
        Hooks.effect(
          OnMount,
          () => Some(() => Overlay.clearMenu(id)),
          hooks,
        );

      switch (maybeRef) {
      | Some(node) =>
        let (x, y, width, _) =
          Math.BoundingBox2d.getBounds(node#getBoundingBox());

        let x =
          switch (orientation) {
          | (_, `Left) => x
          | (_, `Middle) => x -. width /. 2.
          | (_, `Right) => x -. width
          };

        let x = int_of_float(x) + offsetX;
        let y = int_of_float(y) + offsetY;

        Overlay.setMenu(
          id,
          <Menu items x y orientation theme font onItemSelect />,
        );

      | None => ()
      };

      (<View ref={node => setRef(_ => Some(node))} />, hooks);
    });
};

include Anchor;
