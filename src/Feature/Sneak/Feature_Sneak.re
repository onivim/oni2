open Revery.Math;

module StringEx = Oni_Core.Utility.StringEx;

type callback = unit => unit;

type sneakInfo = {
  callback,
  boundingBox: BoundingBox2d.t,
};

type sneak = {
  callback,
  boundingBox: BoundingBox2d.t,
  id: string,
};

type model = {
  active: bool,
  allSneaks: list(sneak),
  inputText: string,
  filteredSneaks: list(sneak),
};

let initial: model = {
  active: false,
  allSneaks: [],
  inputText: "",
  filteredSneaks: [],
};

let prefix = ({inputText, _}) => inputText;

let getTextHighlight = (text: string, model) => {
  let prefix = model |> prefix;
  let prefixLength = prefix |> String.length;
  let idLength = text |> String.length;

  let remainder = String.sub(text, prefixLength, idLength - prefixLength);

  (prefix, remainder);
};

let reset = _sneak => {...initial, active: true};

let hide = _sneak => initial;

let isActive = sneaks => sneaks.active;

module Internal = {
  let filter = (prefix: string, sneak: sneak) => {
    StringEx.startsWith(~prefix, sneak.id);
  };

  let applyFilter = (sneaks: model) => {
    let prefix = sneaks |> prefix;
    if (prefix == "") {
      {...sneaks, filteredSneaks: sneaks.allSneaks};
    } else {
      {
        ...sneaks,
        filteredSneaks: List.filter(filter(prefix), sneaks.allSneaks),
      };
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

  if (String.length(characterToAdd) == 1) {
    let inputText = sneaks.inputText ++ characterToAdd;
    {...sneaks, inputText} |> Internal.applyFilter;
  } else if (characterToAdd == "<BS>" || characterToAdd == "<C-H>") {
    let inputText =
      String.length(sneaks.inputText) >= 1
        ? String.sub(
            sneaks.inputText,
            0,
            String.length(sneaks.inputText) - 1,
          )
        : sneaks.inputText;
    {...sneaks, inputText} |> Internal.applyFilter;
  } else {
    sneaks;
  };
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
    id: int,
    boundingBoxRef: ref(option(Revery.Math.BoundingBox2d.t)),
    callback: unit => unit,
  };

  module MutableState = {
    let nextId = ref(0);
    let singleton = ref([]);
  };

  let register = (boundingBoxRef, callback) => {
    let id = MutableState.nextId^;
    incr(MutableState.nextId);

    MutableState.singleton :=
      [{id, boundingBoxRef, callback}, ...MutableState.singleton^];
    id;
  };

  let unregister = id => {
    let filter = sneakInfo => sneakInfo.id !== id;
    MutableState.singleton := List.filter(filter, MutableState.singleton^);
  };

  let getSneaks = () => {
    MutableState.singleton^
    |> List.filter_map(({boundingBoxRef, callback, _}) => {
         boundingBoxRef^ |> Option.map(boundingBox => {callback, boundingBox})
       });
  };
};

// UPDATE

[@deriving show({with_path: false})]
type command =
  | Start
  | Stop;

[@deriving show({with_path: false})]
type msg =
  | NoneAvailable
  | Command(command)
  | Executed([@opaque] sneak)
  | Discovered([@opaque] list(sneakInfo))
  | KeyboardInput(string);

// EFFECTS

module Effects = {
  let discoverSneak =
    Isolinear.Effect.createWithDispatch(~name="sneak.discover", dispatch => {
      let sneaks = Registry.getSneaks();
      dispatch(Discovered(sneaks));
    });

  let completeSneak = model =>
    Isolinear.Effect.createWithDispatch(~name="sneak.discover", dispatch => {
      let filteredSneaks = getFiltered(model);

      switch (filteredSneaks) {
      | [] => dispatch(NoneAvailable)
      | [sneak] =>
        let {callback, _}: sneak = sneak;
        callback();
        dispatch(Executed(sneak));
      | _ => ()
      };
    });
};

// COMMANDS

module Commands = {
  open Feature_Commands.Schema;

  let start =
    define(
      ~category="Sneak",
      ~title="Enter sneak mode (keyboard-accessible UI)",
      "sneak.start",
      Command(Start),
    );

  let stop =
    define(
      ~category="Sneak",
      ~title="Exit sneak mode",
      "sneak.stop",
      Command(Stop),
    );
};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update = (model, action) => {
  switch (action) {
  | Command(Start) => (reset(model), Effect(Effects.discoverSneak))
  | Command(Stop) => (hide(model), Nothing)
  | Executed(_)
  | NoneAvailable => (hide(model), Nothing)
  | KeyboardInput(k) =>
    let newState = refine(k, model);
    (newState, Effect(Effects.completeSneak(newState)));
  | Discovered(sneaks) => (add(sneaks, model), Nothing)
  //    | _ => default
  };
};

// VIEW

module View = {
  open Revery.UI;
  open Revery.UI.Components;

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

    let text = theme => [color(Colors.Oni.Sneak.foreground.from(theme))];
    let highlight = theme => [
      backgroundColor(Colors.Oni.Sneak.background.from(theme)),
      color(Colors.Oni.Sneak.highlight.from(theme)),
    ];
  };

  module Overlay = {
    let make = (~model, ~theme, ~font: Oni_Core.UiFont.t, ()) => {
      let makeSneak = (bbox, text) => {
        let (x, y, _width, _height) = BoundingBox2d.getBounds(bbox);

        let (highlightText, remainingText) = getTextHighlight(text, model);
        <View style={Styles.item(int_of_float(x), int_of_float(y), theme)}>
          <Text
            style={Styles.highlight(theme)}
            text=highlightText
            fontFamily={font.family}
            fontSize={font.size}
          />
          <Text
            style={Styles.text(theme)}
            text=remainingText
            fontFamily={font.family}
            fontSize={font.size}
          />
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
                    ~sneakId,
                    ~style=[],
                    ~onClick=() => (),
                    ~onRightClick=() => (),
                    ~onAnyClick=_ => (),
                    ~onDoubleClick=() => (),
                    ~onSneak=?,
                    ~onBlur=?,
                    ~onFocus=?,
                    ~tabindex=?,
                    ~onKeyDown=?,
                    ~onKeyUp=?,
                    ~onMouseEnter=?,
                    ~onMouseLeave=?,
                    ~onTextEdit=?,
                    ~onTextInput=?,
                    ~children,
                    (),
                  ) => {
      let%hook bboxRef = Hooks.ref(None);

      let%hook () =
        Hooks.effect(
          OnMountAndIf((!=), sneakId),
          () => {
            let maybeId =
              switch (onSneak) {
              | Some(cb) => Some(Registry.register(bboxRef, cb))
              | None => Some(Registry.register(bboxRef, onClick))
              };

            Some(
              () => {maybeId |> Option.iter(id => Registry.unregister(id))},
            );
          },
        );

      <Clickable
        style
        onClick
        onRightClick
        onAnyClick
        onDoubleClick
        onBoundingBoxChanged={bbox => bboxRef := Some(bbox)}
        ?onBlur
        ?onFocus
        ?tabindex
        ?onKeyDown
        ?onKeyUp
        ?onMouseEnter
        ?onMouseLeave
        ?onTextEdit
        ?onTextInput>
        children
      </Clickable>;
    };
  };
};

module Contributions = {
  let commands = Commands.[start, stop];
};
