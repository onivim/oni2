open Revery.Math;

module InputModel = Oni_Components.InputModel;
module StringEx = Oni_Core.Utility.StringEx;
module Selection = Oni_Components.Selection;

type callback = unit => unit;
type bounds = unit => option(BoundingBox2d.t);

type sneakInfo = {
  callback,
  boundingBox: BoundingBox2d.t,
};

type sneak = {
  callback,
  boundingBox: BoundingBox2d.t,
  id: string,
};

[@deriving show({with_path: false})]
type msg =
  | NoneAvailable
  | Executed([@opaque] sneak)
  | Discovered([@opaque] list(sneakInfo))
  | KeyboardInput(string);

type model = {
  active: bool,
  allSneaks: list(sneak),
  prefix: string,
  filteredSneaks: list(sneak),
};

let initial: model = {
  active: false,
  allSneaks: [],
  prefix: "",
  filteredSneaks: [],
};

let getTextHighlight = (text: string, model) => {
  let prefixLength = model.prefix |> String.length;
  let idLength = text |> String.length;

  let remainder = String.sub(text, prefixLength, idLength - prefixLength);

  (model.prefix, remainder);
};

let reset = _sneak => {...initial, active: true};

let hide = _sneak => initial;

let isActive = sneaks => sneaks.active;

module Internal = {
  let filter = (prefix: string, sneak: sneak) => {
    StringEx.startsWith(~prefix, sneak.id);
  };

  let applyFilter = (sneaks: model) =>
    if (sneaks.prefix == "") {
      {...sneaks, filteredSneaks: sneaks.allSneaks};
    } else {
      {
        ...sneaks,
        filteredSneaks: List.filter(filter(sneaks.prefix), sneaks.allSneaks),
      };
    };

  // Ported from: https://github.com/onivim/oni/blob/74a4dc7f2240a1f5f7a799b2f3f9d01d69b01bac/browser/src/Services/Sneak/SneakStore.ts#L95
  // But could be improved:
  // - Preference for home row
  let getLabelFromIndex = (i: int) => {
    let aChar = Char.code('A');
    let firstDigit = i / 26;
    let secondDigit = i - firstDigit * 26;

    let firstChar = Char.chr(firstDigit + aChar);
    let secondChar = Char.chr(secondDigit + aChar);
    String.make(1, firstChar) ++ String.make(1, secondChar);
  };
};

let refine = (characterToAdd: string, sneaks: model) => {
  let characterToAdd = String.uppercase_ascii(characterToAdd);
  let selection =
    String.length(sneaks.prefix) |> Selection.collapsed(~text=sneaks.prefix);

  let (prefix, _) =
    InputModel.handleInput(~text=sneaks.prefix, ~selection, characterToAdd);

  {...sneaks, prefix} |> Internal.applyFilter;
};

let add = (sneaksToAdd: list(sneakInfo), sneaks: model) => {
  let toSneakInfo = (index: int, sneak: sneakInfo) => {
    {
      boundingBox: sneak.boundingBox,
      callback: sneak.callback,
      id: Internal.getLabelFromIndex(index),
    };
  };

  let sort = (sneakA: sneakInfo, sneakB: sneakInfo) => {
    let bboxA = sneakA.boundingBox;
    let bboxB = sneakB.boundingBox;

    let (aX, aY, _, _) = Revery.Math.BoundingBox2d.getBounds(bboxA);
    let (bX, bY, _, _) = Revery.Math.BoundingBox2d.getBounds(bboxB);

    aX -. bX +. (aY -. bY) |> int_of_float;
  };

  let allSneaks = sneaksToAdd |> List.sort(sort) |> List.mapi(toSneakInfo);
  let filteredSneaks = allSneaks;

  {...sneaks, allSneaks, filteredSneaks};
};

let getFiltered = ({filteredSneaks, _}) => filteredSneaks;

// REGISTRY
module Registry = {
  type sneakInfo = {
    node: ref(option(Revery.UI.node)),
    callback: unit => unit,
  };

  module MutableState = {
    let singleton = ref([]);
  };

  let register = (node: ref(option(Revery.UI.node)), callback) => {
    MutableState.singleton := [{node, callback}, ...MutableState.singleton^];
  };

  let unregister = (node: ref(option(Revery.UI.node))) => {
    let filter = sneakInfo => sneakInfo.node !== node;
    MutableState.singleton := List.filter(filter, MutableState.singleton^);
  };

  let getSneaks = () => {
    MutableState.singleton^
    |> List.filter_map(item => {
         switch (item.node^) {
         | Some(node) => Some((node, item.callback))
         | None => None
         }
       })
    |> List.map(((node, callback)) => {
         {callback, boundingBox: node#getBoundingBox()}
       });
  };
};

module View = {
  open Revery;
  open Revery.UI;
  open Revery.UI.Components;
  open Oni_Components;

  module Colors = Feature_Theme.Colors;

  module Constants = {
    let size = 25;
  };

  module Styles = {
    open Style;

    let backdrop = theme => [
      backgroundColor(Colors.Oni.Modal.backdrop.from(theme)),
      position(`Absolute),
      top(0),
      left(0),
      right(0),
      bottom(0),
    ];

    let item = (x, y, theme) => [
      backgroundColor(Colors.Oni.Sneak.background.from(theme)),
      position(`Absolute),
      boxShadow(
        ~xOffset=3.,
        ~yOffset=3.,
        ~blurRadius=5.,
        ~spreadRadius=0.,
        ~color=Revery.Color.rgba(0., 0., 0., 0.2),
      ),
      top(y),
      left(x + Constants.size / 2),
      Style.height(Constants.size),
      Style.width(Constants.size),
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
    ];

    let text = (theme, font: Oni_Core.UiFont.t) => [
      color(Colors.Oni.Sneak.foreground.from(theme)),
      fontFamily(font.fontFile),
      fontSize(12.),
    ];
    let highlight = (theme, font: Oni_Core.UiFont.t) => [
      backgroundColor(Colors.Oni.Sneak.background.from(theme)),
      color(Colors.Oni.Sneak.highlight.from(theme)),
      fontFamily(font.fontFile),
      fontSize(12.),
    ];
  };

  module SneakOverlay = {
    let make = (~model, ~theme, ~font, ()) => {
      let makeSneak = (bbox, text) => {
        let (x, y, _width, _height) = BoundingBox2d.getBounds(bbox);

        let (highlightText, remainingText) = getTextHighlight(text, model);
        <View style={Styles.item(int_of_float(x), int_of_float(y), theme)}>
          <Text style={Styles.highlight(theme, font)} text=highlightText />
          <Text style={Styles.text(theme, font)} text=remainingText />
        </View>;
      };

      let sneaks = getFiltered(model);
      let sneakViews =
        List.map(
          ({boundingBox, id, _}) => makeSneak(boundingBox, id),
          sneaks,
        )
        |> React.listToElement;

      let isActive = isActive(model);
      isActive
        ? <View style={Styles.backdrop(theme)}> sneakViews </View>
        : React.empty;
    };
  };

  module Sneakable = {
    let%component make =
                  (
                    ~style=[],
                    ~onClick=() => (),
                    ~onRightClick=() => (),
                    ~onAnyClick=_ => (),
                    ~onSneak=?,
                    ~onBlur=?,
                    ~onFocus=?,
                    ~tabindex=?,
                    ~onKeyDown=?,
                    ~onKeyUp=?,
                    ~onTextEdit=?,
                    ~onTextInput=?,
                    ~children,
                    (),
                  ) => {
      let%hook (holder: ref(option(Revery.UI.node)), _) =
        Hooks.state(ref(None));

      let componentRef = (node: Revery.UI.node) => {
        holder := Some(node);
      };

      let%hook () =
        Hooks.effect(
          OnMount,
          () => {
            switch (onSneak) {
            | Some(cb) => Registry.register(holder, cb)
            | None => Registry.register(holder, onClick)
            };

            Some(
              () => {
                holder := None;
                Registry.unregister(holder);
              },
            );
          },
        );

      <Clickable
        style
        onClick
        onRightClick
        onAnyClick
        componentRef
        ?onBlur
        ?onFocus
        ?tabindex
        ?onKeyDown
        ?onKeyUp
        ?onTextEdit
        ?onTextInput>
        children
      </Clickable>;
    };
  };
};
