open Oni_Core;

open Revery;
open Revery.UI;

module Colors = Feature_Theme.Colors;

module Constants = {
  let menuWidth = 225;
  // let maxMenuHeight = 600;

  let overlayY = 30;
};

// MODEL
[@deriving show({with_path: false})]
type item('data) = {
  label: string,
  // icon: option(IconTheme.IconDefinition.t),
  data: [@opaque] 'data,
  details: [@opaque] element,
};

[@deriving show({with_path: false})]
type content('data) =
  | Item(item('data))
  | Group(list(content('data)))
  | Submenu({
      label: string,
      items: list(content('data)),
    });

[@deriving show]
type submenu('data) = {
  items: list(content('data)),
  yOffset: float,
};

[@deriving show]
type msg('data) =
  | ItemClicked({data: 'data})
  | ClickedOutside
  | SubmenuHovered({revSubmenus: list(submenu('data))});

type outmsg('data) =
  | Nothing
  | Selected({data: 'data})
  | Cancelled;

type model('data) = {
  items: list(content('data)),
  openSubmenus: list(submenu('data)),
};

let make = items => {items, openSubmenus: []};

let update = (msg, model) => {
  switch (msg) {
  | ItemClicked({data}) => (model, Selected({data: data}))
  | ClickedOutside => (model, Cancelled)
  | SubmenuHovered({revSubmenus}) => (
      {...model, openSubmenus: List.rev(revSubmenus)},
      Nothing,
    )
  };
};

let map = (~f: item('a) => item('a), {items, openSubmenus}) => {
  let rec loopItems =
    fun
    | Item(data) => Item(f(data))
    | Group(items) => Group(List.map(loopItems, items))
    | Submenu({label, items}) =>
      Submenu({label, items: List.map(loopItems, items)});

  let items' = List.map(loopItems, items);

  let subMenus' =
    List.map(
      (submenu: submenu('a)) =>
        {
          yOffset: submenu.yOffset,
          items: List.map(loopItems, submenu.items),
        },
      openSubmenus,
    );
  {items: items', openSubmenus: subMenus'};
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
        padding(4),
        flexDirection(`Row),
        alignItems(`Center),
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
      (
        ~label: string,
        ~details: Revery.UI.element,
        ~theme: ColorTheme.Colors.t,
        ~font: UiFont.t,
        ~onClick: unit => unit,
        ~onHover: Revery.Math.BoundingBox2d.t => unit,
        unit
      ) =>
      _ =
      (~label, ~details, ~theme, ~font, ~onClick, ~onHover, ()) =>
        component(hooks => {
          let ((isFocused, setIsFocused), hooks) =
            Hooks.state(false, hooks);
          let ((maybeBbox, setBbox), hooks) = Hooks.state(None, hooks);

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
              text=label
            />;
          };

          (
            <View onMouseUp={_ => {onClick()}}>
              <View
                style={Styles.container(~theme, ~isFocused)}
                onBoundingBoxChanged={(bbox: Math.BoundingBox2d.t) => {
                  setBbox(_ => Some(bbox))
                }}
                onMouseOut={_ => setIsFocused(_ => false)}
                onMouseOver={_ => {
                  Option.iter(bbox => onHover(bbox), maybeBbox);
                  setIsFocused(_ => true);
                }}>
                // iconView

                  <View
                    style=Style.[marginLeft(4), flexGrow(1), flexShrink(1)]>
                    labelView
                  </View>
                  <View
                    style=Style.[
                      marginHorizontal(4),
                      flexGrow(0),
                      flexShrink(0),
                    ]>
                    details
                  </View>
                </View>
            </View>,
            hooks,
          );
        });
  };

  let divider = (~theme, ()) => {
    let fg = Colors.Menu.foreground.from(theme);
    <View
      style=Style.[
        marginHorizontal(8),
        height(1),
        opacity(0.3),
        backgroundColor(fg),
      ]
    />;
  };

  // MENU

  module Submenu = {
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

    let subMenuIcon = (~theme, ()) => {
      let icon = Codicon.triangleRight;
      let color = Colors.Menu.foreground.from(theme);
      <Codicon icon fontSize=12. color />;
    };

    let component = React.Expert.component("Submenu");
    let make =
        (~items, ~x, ~y, ~theme, ~font, ~onItemSelect, ~onSubmenuHover, ()) =>
      component(hooks => {
        (
          <View style={Styles.container(~x, ~y, ~theme)}>
            {items
             |> List.mapi((idx, item) => {
                  switch (item) {
                  | Submenu({label, items}) =>
                    let onClick = () => ();
                    let onHover = bbox => {
                      let (_x, yOffset, _width, _height) =
                        Revery.Math.BoundingBox2d.getBounds(bbox);
                      onSubmenuHover(Some({yOffset, items}));
                    };
                    let details = <subMenuIcon theme />;
                    [<MenuItem label theme font onClick onHover details />];

                  | Item(item) =>
                    let onClick = () => onItemSelect(item);
                    let onHover = _ => onSubmenuHover(None);
                    [
                      <MenuItem
                        label={item.label}
                        details={item.details}
                        theme
                        font
                        onClick
                        onHover
                      />,
                    ];

                  | Group(items) =>
                    let items' =
                      items
                      |> List.map(item => {
                           let elem =
                             switch (item) {
                             | Group(_) => []
                             | Submenu({label, items}) =>
                               let onClick = () => ();
                               let onHover = bbox => {
                                 let (_x, yOffset, _width, _height) =
                                   Revery.Math.BoundingBox2d.getBounds(bbox);
                                 onSubmenuHover(Some({yOffset, items}));
                               };
                               let details = <subMenuIcon theme />;
                               [
                                 <MenuItem
                                   details
                                   label
                                   theme
                                   font
                                   onClick
                                   onHover
                                 />,
                               ];

                             | Item(item) =>
                               let onClick = () => onItemSelect(item);
                               let onHover = _ => onSubmenuHover(None);
                               [
                                 <MenuItem
                                   label={item.label}
                                   details={item.details}
                                   theme
                                   font
                                   onClick
                                   onHover
                                 />,
                               ];
                             };
                           elem;
                         })
                      |> List.flatten;

                    idx == 0 ? items' : [<divider theme />, ...items'];
                  }
                })
             |> List.flatten
             |> React.listToElement}
          </View>,
          hooks,
        )
      });
  };

  module Menu = {
    module Styles = {
      open Style;

      let container = [
        position(`Absolute),
        top(0),
        left(0),
        bottom(0),
        right(0),
      ];
    };

    let component = React.Expert.component("Menu");
    let make =
        (~model, ~x, ~y, ~theme, ~font, ~onItemSelect, ~onSubmenuHover, ()) =>
      component(hooks => {
        let rootMenu =
          <Submenu
            items={model.items}
            x
            y
            theme
            font
            onItemSelect
            onSubmenuHover={maybeItems => {
              (
                switch (maybeItems) {
                | None => []
                | Some(v) => [v]
                }
              )
              |> onSubmenuHover
            }}
          />;

        let (menus, _, _) =
          model.openSubmenus
          |> List.fold_left(
               (acc, curr: submenu('a)) => {
                 let (menus, submenus, xOffset) = acc;

                 let newX = xOffset + Constants.menuWidth;
                 let newSubmenus = [curr, ...submenus];
                 let nextMenu =
                   <Submenu
                     items={curr.items}
                     x=newX
                     y={int_of_float(curr.yOffset) - Constants.overlayY}
                     theme
                     font
                     onItemSelect
                     onSubmenuHover={maybeItems =>
                       (
                         switch (maybeItems) {
                         | None => newSubmenus
                         | Some(item) => [item, ...newSubmenus]
                         }
                       )
                       |> onSubmenuHover
                     }
                   />;

                 ([nextMenu, ...menus], newSubmenus, newX);
               },
               ([rootMenu], [], x),
             );

        let menuElements = menus |> React.listToElement;

        (<View style=Styles.container> menuElements </View>, hooks);
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

      let onOverlayClick = _ => {
        IntMap.iter((_key, {onCancel, _}) => {onCancel()}, menus);
      };

      if (IntMap.is_empty(menus)) {
        React.empty;
      } else {
        <View onMouseUp=onOverlayClick style=Styles.backdrop>
          {IntMap.bindings(menus)
           |> List.map(snd)
           |> List.map(({menu, _}) => menu)
           |> React.listToElement}
        </View>;
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
        let onSubmenuHover = revSubmenus => {
          dispatch(SubmenuHovered({revSubmenus: revSubmenus}));
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
        | Some(bbox: Math.BoundingBox2d.t) =>
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
            <Menu model x y theme font onItemSelect onSubmenuHover />,
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
