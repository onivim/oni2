open Oni_Core;

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Constants = {
  let menuWidth = 200;
  // let maxMenuHeight = 600;
};

// TYPES

module Id: {
  type t;
  let create: unit => t;
} = {
  type t = int;

  let lastId = ref(0);
  let create = () => {
    incr(lastId);
    lastId^;
  };
};

[@deriving show({with_path: false})]
type item('data) = {
  label: string,
  // icon: option(IconTheme.IconDefinition.t),
  data: [@opaque] 'data,
};

type placement = {
  x: int,
  y: int,
  orientation: ([ | `Top | `Middle | `Bottom], [ | `Left | `Middle | `Right]),
};

type t('data) = {
  id: Id.t,
  placement: option(placement),
  items: list(item('data)),
};

// MENUITEM

module MenuItem = {
  module Constants = {
    let fontSize = 12.;
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
  };

  let component = React.Expert.component("MenuItem");
  let make:
    'data.
    (
      ~item: item('data),
      ~theme: Theme.t,
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
          let style = Styles.label(~font, ~theme, ~isFocused);
          <Text style text={item.label} />;
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

    let container = (~x, ~y, ~theme: Theme.t) => [
      position(`Absolute),
      top(y),
      left(x),
      backgroundColor(theme.menuBackground),
      color(theme.menuForeground),
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
  let make = (~items, ~placement, ~theme, ~font, ~onItemSelect, ()) =>
    component(hooks => {
      let ((maybeRef, setRef), hooks) = Hooks.state(None, hooks);
      let {x, y, orientation: (orientY, orientX)} = placement;

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
  module Styles = {
    open Style;

    let overlay = [
      position(`Absolute),
      top(0),
      bottom(0),
      left(0),
      right(0),
      pointerEvents(`Allow),
      cursor(MouseCursors.arrow),
    ];
  };

  let make = (~model, ~theme, ~font, ~onOverlayClick, ~onItemSelect, ()) =>
    switch (model) {
    | {items, placement: Some(placement), _} =>
      <Clickable onClick=onOverlayClick style=Styles.overlay>
        <Menu items placement theme font onItemSelect />
      </Clickable>
    | _ => React.empty
    };
};

// INSTANCE

module Make = (()) => {
  let id = Id.create();

  let init = items => {id, placement: None, items};

  // ANCHOR

  module Anchor = {
    let component = React.Expert.component("Anchor");
    let make =
        (
          ~model as maybeModel,
          ~orientation=(`Bottom, `Left),
          ~offsetX=0,
          ~offsetY=0,
          ~onUpdate,
          (),
        ) =>
      component(hooks => {
        let ((maybeRef, setRef), hooks) = Hooks.ref(None, hooks);

        switch (maybeModel, maybeRef) {
        | (Some(model), Some(node)) =>
          if (model.id == id) {
            let (x, y, width, _) =
              Math.BoundingBox2d.getBounds(node#getBoundingBox());

            let x =
              switch (orientation) {
              | (_, `Left) => x
              | (_, `Middle) => x -. width /. 2.
              | (_, `Right) => x -. width
              };

            let placement =
              Some({
                x: int_of_float(x) + offsetX,
                y: int_of_float(y) + offsetY,
                orientation,
              });

            if (model.placement != placement) {
              onUpdate({...model, placement});
            };
          }

        | _ => ()
        };

        (<View ref={node => setRef(Some(node))} />, hooks);
      });
  };
};
