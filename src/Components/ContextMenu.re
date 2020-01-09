open Oni_Core;

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Option = Utility.Option;

module Constants = {
  let menuWidth = 200;
  // let maxMenuHeight = 600;
};

// TYPES

type identity = int;

type item('data) = {
  label: string,
  // icon: option(IconTheme.IconDefinition.t),
  data: 'data,
};

type t('data) = {
  identity,
  x: int,
  y: int,
  originX: [ | `Left | `Middle | `Right],
  originY: [ | `Top | `Middle | `Bottom],
  items: list(item('data)),
};

let create = (~originX=`Left, ~originY=`Bottom, identity, items) => {
  identity,
  x: 0,
  y: 0,
  originX,
  originY,
  items,
};

// MENUITEM

module MenuItem = {
  module Constants = {
    let fontSize = 12;
  };

  module Styles = {
    open Style;

    let bg = (~theme: Theme.t, ~isFocused) =>
      isFocused ? theme.menuSelectionBackground : theme.menuBackground;

    let container = (~theme, ~isFocused) => [
      padding(10),
      flexDirection(`Row),
      backgroundColor(bg(~theme, ~isFocused)),
    ];

    // let icon = fgColor => [
    //   fontFamily("seti.ttf"),
    //   fontSize(Constants.fontSize),
    //   marginRight(10),
    //   color(fgColor),
    // ];

    let label = (~font: UiFont.t, ~theme: Theme.t, ~isFocused) => [
      fontFamily(font.fontFile),
      textOverflow(`Ellipsis),
      fontSize(Constants.fontSize),
      color(theme.menuForeground),
      backgroundColor(bg(~theme, ~isFocused)),
    ];

    let clickable = [cursor(Revery.MouseCursors.pointer)];
  };

  let make:
    'data.
    (
      ~item: item('data),
      ~theme: Theme.t,
      ~font: UiFont.t,
      ~onClick: unit => unit,
      unit
    ) =>
    React.element(React.node)
   =
    (~item, ~theme, ~font, ~onClick, ()) => {
      let isFocused = false;

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
        let style = Styles.label(~font, ~theme, ~isFocused);
        <Text style text={item.label} />;
      };

      <Clickable style=Styles.clickable onClick>
        // onMouseOver={_ => setIsFocused(_ => true)}
        // onMouseOut={_ => setIsFocused(_ => false)}

          <View style={Styles.container(~theme, ~isFocused)}>
            // iconView
             labelView </View>
        </Clickable>;
    };
};

// MENU

module Menu = {
  module Styles = {
    open Style;

    let container = (~x, ~y, ~theme: Theme.t) => [
      position(`Absolute),
      top(y),
      left(x),
      backgroundColor(theme.menuBackground),
      color(theme.menuForeground),
      width(Constants.menuWidth),
    ];
  };

  let component = React.Expert.component("Menu");
  let make = (~model, ~theme, ~font, ~onItemSelect, ()) =>
    component(hooks => {
      let ((maybeRef, setRef), hooks) = Hooks.state(None, hooks);
      let {x, y, originX, originY, items, _} = model;

      let height =
        switch (maybeRef) {
        | Some((node: node)) => node#measurements().height
        | None => List.length(model.items) * 20
        };
      let width = Constants.menuWidth;

      let x =
        switch (originX) {
        | `Left => x
        | `Middle => x - width / 2
        | `Right => x - width
        };

      let y =
        switch (originY) {
        | `Top => y - height
        | `Middle => y - height / 2
        | `Bottom => y
        };

      (
        <BoxShadow
          boxShadow={Style.BoxShadow.make(
            ~xOffset=-11.,
            ~yOffset=-11.,
            ~blurRadius=25.,
            ~spreadRadius=0.,
            ~color=Color.rgba(0., 0., 0., 0.2),
            (),
          )}>
          <View
            style={Styles.container(~x, ~y, ~theme)}
            ref={node => setRef(_ => Some(node))}>
            {items
             |> List.map(item => {
                  let onClick = () => onItemSelect(item);
                  <MenuItem item theme font onClick />;
                })
             |> React.listToElement}
          </View>
        </BoxShadow>,
        hooks,
      );
    });
};

// OVERLAY

module Overlay = {
  module Styles = {
    open Style;

    let overlay = [
      position(`Absolute),
      top(0),
      bottom(0),
      left(0),
      right(0),
      pointerEvents(`Allow),
    ];
  };

  let make = (~model, ~theme, ~font, ~onOverlayClick, ~onItemSelect, ()) => {
    <Clickable onClick=onOverlayClick style=Styles.overlay>
      <Menu model theme font onItemSelect />
    </Clickable>;
  };
};

// IDENTITY

module Identity = {
  let generateId = {
    let lastId = ref(0);
    () => {
      lastId := lastId^ + 1;
      lastId^;
    };
  };

  let component = React.Expert.component("Anchor");
  let make = (~children as render, ()) =>
    component(hooks => {
      let ((id, _), hooks) = Hooks.state(generateId(), hooks);

      (render(id), hooks);
    });
};

// ANCHOR

module Anchor = {
  let component = React.Expert.component("Anchor");
  let make = (~identity, ~model as maybeModel, ~onUpdate, ()) =>
    component(hooks => {
      let ((maybeRef, setRef), hooks) = Hooks.ref(None, hooks);

      switch (maybeModel, maybeRef) {
      | (Some(model), Some(node)) =>
        if (model.identity == identity) {
          let (x, y, _, _) =
            Math.BoundingBox2d.getBounds(node#getBoundingBox());
          let (x, y) = (int_of_float(x), int_of_float(y));

          if (x != model.x || y != model.y) {
            onUpdate({...model, x, y});
          };
        }
      | _ => ()
      };

      (<View ref={node => setRef(Some(node))} />, hooks);
    });
};
