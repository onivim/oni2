open Oni_Core;

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Colors = Feature_Theme.Colors;

module Constants = {
  let menuWidth = 200;
  // let maxMenuHeight = 600;

  let overlayY = 30;
};

// MODEL
[@deriving show({with_path: false})]
type item('data) = {
  label: string,
  // icon: option(IconTheme.IconDefinition.t),
  data: [@opaque] 'data,
};

[@deriving show]
type msg('data) =
  | ItemClicked({data: 'data})
  | ClickedOutside;

type outmsg('data) =
  | Nothing
  | Selected({data: 'data})
  | Cancelled;

type model('data) = {items: list(item('data))};

let make = items => {items: items};

let update = (msg, model) => {
  switch (msg) {
  | ItemClicked({data}) => (model, Selected({data: data}))
  | ClickedOutside => (model, Cancelled)
  };
};

module View = {
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
          let ((isFocused, setIsFocused), hooks) =
            Hooks.state(false, hooks);

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
    let make =
        (~items, ~x, ~y, ~orientation, ~theme, ~font, ~onItemSelect, ()) =>
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
  type menuInfo = {
    menu: Revery.UI.element,
    onCancel: unit => unit,
  };

  module Overlay = {
    let internalSetMenus = ref(_ => ());

    let setMenu = (id, menu, onCancel) =>
      internalSetMenus^(IntMap.add(id, {menu, onCancel}));

    let clearMenu = id => internalSetMenus^(IntMap.remove(id));

    module Styles = {
      open Style;

      let backdrop = [
        position(`Absolute),
        top(Constants.overlayY), // TODO
        bottom(0),
        left(0),
        right(0),
        pointerEvents(`Allow),
        cursor(MouseCursors.arrow),
      ];
    };

    let%component make = () => {
      let%hook (menus: IntMap.t(menuInfo), setMenus) =
        Hooks.state(IntMap.empty);
      internalSetMenus := setMenus;

      let onOverlayClick = () => {
        IntMap.iter((_key, {onCancel, _}) => {onCancel()}, menus);
      };

      if (IntMap.is_empty(menus)) {
        React.empty;
      } else {
        <Clickable onClick=onOverlayClick style=Styles.backdrop>
          {IntMap.bindings(menus)
           |> List.map(snd)
           |> List.map(({menu, _}) => menu)
           |> React.listToElement}
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
          ~model: model('a),
          ~orientation=(`Bottom, `Left),
          ~offsetX=0,
          ~offsetY=0,
          // ~onItemSelect,
          ~dispatch: msg('a) => unit,
          // ~onCancel,
          ~theme,
          ~font,
          (),
        ) =>
      component(hooks => {
        let onCancel = () => dispatch(ClickedOutside);

        let onItemSelect = item => {
          dispatch(ItemClicked({data: item.data}));
        };
        let ((id, _), hooks) = Hooks.state(generateId(), hooks);
        let ((maybeBbox, setBbox), hooks) = Hooks.state(None, hooks);
        let ((), hooks) =
          Hooks.effect(
            OnMount,
            () => Some(() => Overlay.clearMenu(id)),
            hooks,
          );

        switch (maybeBbox) {
        | Some((bbox: Math.BoundingBox2d.t)) =>
          let (x, y, width, height) = bbox |> Math.BoundingBox2d.getBounds;

          let x =
            switch (orientation) {
            | (_, `Left) => x
            | (_, `Middle) => x -. width /. 2.
            | (_, `Right) => x -. width
            };

          let y =
            switch (orientation) {
            | (`Top, _) => y
            | (`Middle, _) => y +. height /. 2.
            | (`Bottom, _) => y
            };

          let x = int_of_float(x) + offsetX;
          let y = int_of_float(y) + offsetY - Constants.overlayY;

          Overlay.setMenu(
            id,
            <Menu
              items={model.items}
              x
              y
              orientation
              theme
              font
              onItemSelect
            />,
            onCancel,
          );

        | None => ()
        };

        (
          <View
            onBoundingBoxChanged={(bbox: Math.BoundingBox2d.t) => {
              setBbox(_ => Some(bbox))
            }}
          />,
          hooks,
        );
      });
  };

  include Anchor;
};
