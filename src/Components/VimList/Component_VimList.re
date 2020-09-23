open Oni_Core;
open Utility;

// MODEL

[@deriving show]
type model('item) = {
  scrollY: float,
  rowHeight: int,
  items: array('item),
  hovered: option(int),
  selected: int,
  viewportHeight: int,
  viewportWidth: int,
  multiplier: int,
  initialRowsToRender: int,
  // For mouse gestures, we don't want the scroll to be animated - it feels laggy
  // But for keyboarding gestures, like 'zz', the animation is helpful.
  isScrollAnimated: bool,
};

let create = (~rowHeight) => {
  scrollY: 0.,
  rowHeight,
  items: [||],
  hovered: None,
  selected: 0,
  multiplier: 0,
  viewportHeight: 1,
  viewportWidth: 1,
  initialRowsToRender: 10,
  isScrollAnimated: false,
};

let isScrollAnimated = ({isScrollAnimated, _}) => isScrollAnimated;

let findIndex = (f, {items, _}) => {
  let len = Array.length(items);
  let rec loop = idx =>
    if (idx >= len) {
      None;
    } else if (f(items[idx])) {
      Some(idx);
    } else {
      loop(idx + 1);
    };
  loop(0);
};

let selectedIndex = ({selected, _}) => selected;

let resetMultiplier = model => {...model, multiplier: 0};

let applyMultiplierDigit = (digit, model) => {
  ...model,
  multiplier: model.multiplier * 10 + digit,
};

let getMultiplier = ({multiplier, _}) => {
  max(multiplier, 1);
};

let get = (index, {items, _}) => {
  let count = Array.length(items);

  if (index < 0 || index >= count) {
    None;
  } else {
    Some(items[index]);
  };
};

let count = ({items, _}) => Array.length(items);

[@deriving show]
type command =
  | CursorToTop //gg
  | CursorToBottom // G
  | CursorDown // j
  | CursorUp // k
  | Digit(int)
  | Select
  | ScrollCursorCenter // zz
  | ScrollCursorBottom // zb
  | ScrollCursorTop; // zt

[@deriving show]
type msg =
  | Command(command)
  | MouseWheelScrolled({delta: float})
  | ScrollbarMoved({scrollY: float})
  | MouseOver({index: int})
  | MouseOut({index: int})
  | MouseClicked({index: int})
  | ViewDimensionsChanged({
      heightInPixels: int,
      widthInPixels: int,
    });

type outmsg =
  | Nothing
  | Selected({index: int});

let set = (items, model) => {...model, items};

let showTopScrollShadow = ({scrollY, _}) => scrollY > 0.1;
let showBottomScrollShadow = ({items, scrollY, rowHeight, viewportHeight, _}) => {
  let totalHeight = float(Array.length(items) * rowHeight);
  scrollY < totalHeight -. float(viewportHeight);
};

// UPDATE

let ensureSelectedVisible = model => {
  let yPosition = float(model.selected * model.rowHeight);

  let rowHeightF = float(model.rowHeight);
  let viewportHeightF = float(model.viewportHeight);

  let scrollY' =
    if (yPosition < model.scrollY) {
      yPosition;
    } else if (yPosition +. rowHeightF > model.scrollY +. viewportHeightF) {
      yPosition -. (viewportHeightF -. rowHeightF);
    } else {
      model.scrollY;
    };

  {...model, scrollY: scrollY'};
};

let setSelected = (~selected, model) => {
  let count = model.items |> Array.length;
  let selected' =
    if (selected < 0) {
      0;
    } else if (selected >= count) {
      count - 1;
    } else {
      selected;
    };

  {...model, selected: selected'} |> ensureSelectedVisible;
};

let setScrollY = (~scrollY, model) => {
  // Allow for overscroll such that the very last item is visible
  let countMinusOne = max(0, Array.length(model.items) - 1);
  let maxScroll = float(countMinusOne * model.rowHeight);
  let minScroll = 0.;

  let newScrollY = FloatEx.clamp(scrollY, ~hi=maxScroll, ~lo=minScroll);
  {...model, scrollY: newScrollY};
};

let enableScrollAnimation = model => {...model, isScrollAnimated: true};
let disableScrollAnimation = model => {...model, isScrollAnimated: false};

let scrollSelectedToTop = model => {
  model
  |> setScrollY(~scrollY=float(model.selected * model.rowHeight))
  |> enableScrollAnimation;
};

let scrollSelectedToBottom = model => {
  model
  |> setScrollY(
       ~scrollY=
         float(
           model.selected
           * model.rowHeight
           - (model.viewportHeight - model.rowHeight),
         ),
     )
  |> enableScrollAnimation;
};

let scrollSelectedToCenter = model => {
  model
  |> setScrollY(
       ~scrollY=
         float(
           model.selected
           * model.rowHeight
           - (model.viewportHeight - model.rowHeight)
           / 2,
         ),
     )
  |> enableScrollAnimation;
};

let scrollTo = (~index, ~alignment, model) => {
  let model' = model |> setSelected(~selected=index);
  switch (alignment) {
  | `Top => model' |> scrollSelectedToTop
  | `Bottom => model' |> scrollSelectedToBottom
  | `Center => model' |> scrollSelectedToCenter
  | `Reveal => model' |> ensureSelectedVisible
  };
};

let update = (msg, model) => {
  switch (msg) {
  | Command(CursorToTop) => (
      model
      |> setSelected(~selected=0)
      |> enableScrollAnimation
      |> resetMultiplier,
      Nothing,
    )

  | Command(CursorToBottom) =>
    let selected =
      model.multiplier == 0
        ? Array.length(model.items) - 1 : model.multiplier;
    (
      model
      |> setSelected(~selected)
      |> enableScrollAnimation
      |> resetMultiplier,
      Nothing,
    );

  | Command(CursorUp) => (
      model
      |> setSelected(~selected=model.selected - getMultiplier(model))
      |> enableScrollAnimation
      |> resetMultiplier,
      Nothing,
    )

  | Command(CursorDown) => (
      model
      |> setSelected(~selected=model.selected + getMultiplier(model))
      |> enableScrollAnimation
      |> resetMultiplier,
      Nothing,
    )

  | Command(Digit(digit)) => (
      model |> applyMultiplierDigit(digit),
      Nothing,
    )

  | Command(ScrollCursorTop) => (
      model |> scrollSelectedToTop |> resetMultiplier,
      Nothing,
    )

  | Command(ScrollCursorBottom) => (
      model |> scrollSelectedToBottom |> resetMultiplier,
      Nothing,
    )

  | Command(ScrollCursorCenter) => (
      model |> scrollSelectedToCenter |> resetMultiplier,
      Nothing,
    )

  | Command(Select) =>
    let isValidFocus =
      model.selected >= 0 && model.selected < Array.length(model.items);

    if (isValidFocus) {
      (model, Selected({index: model.selected}));
    } else {
      (model, Nothing);
    };

  | MouseClicked({index}) =>
    let isValidIndex = index >= 0 && index < Array.length(model.items);

    if (isValidIndex) {
      (model |> setSelected(~selected=index), Selected({index: index}));
    } else {
      (model, Nothing);
    };

  | MouseWheelScrolled({delta}) => (
      model
      |> setScrollY(~scrollY=model.scrollY +. delta)
      |> disableScrollAnimation,
      Nothing,
    )
  | ScrollbarMoved({scrollY}) => (
      model |> setScrollY(~scrollY) |> disableScrollAnimation,
      Nothing,
    )

  | MouseOver({index}) => ({...model, hovered: Some(index)}, Nothing)

  | MouseOut({index}) =>
    let model' =
      if (Some(index) == model.hovered) {
        {...model, hovered: None};
      } else {
        model;
      };

    (model', Nothing);

  | ViewDimensionsChanged({heightInPixels, widthInPixels}) => (
      {
        ...model,
        viewportWidth: widthInPixels,
        viewportHeight: heightInPixels,
      },
      Nothing,
    )
  };
};

// CONTRIBUTIONS

module Commands = {
  open Feature_Commands.Schema;

  let gg = define("vim.list.gg", Command(CursorToTop));
  let g = define("vim.list.G", Command(CursorToBottom));
  let j = define("vim.list.j", Command(CursorDown));
  let k = define("vim.list.k", Command(CursorUp));
  let enter = define("vim.list.enter", Command(Select));
  let zz = define("vim.list.zz", Command(ScrollCursorCenter));
  let zb = define("vim.list.zb", Command(ScrollCursorBottom));
  let zt = define("vim.list.zt", Command(ScrollCursorTop));

  let digit0 = define("vim.list.0", Command(Digit(0)));
  let digit1 = define("vim.list.1", Command(Digit(1)));
  let digit2 = define("vim.list.2", Command(Digit(2)));
  let digit3 = define("vim.list.3", Command(Digit(3)));
  let digit4 = define("vim.list.4", Command(Digit(4)));
  let digit5 = define("vim.list.5", Command(Digit(5)));
  let digit6 = define("vim.list.6", Command(Digit(6)));
  let digit7 = define("vim.list.7", Command(Digit(7)));
  let digit8 = define("vim.list.8", Command(Digit(8)));
  let digit9 = define("vim.list.9", Command(Digit(9)));
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let vimListNavigation = bool("vimListNavigation", _ => true);
};

module Keybindings = {
  open Oni_Input;

  let commandCondition =
    "!textInputFocus && vimListNavigation" |> WhenExpr.parse;

  let keybindings =
    Keybindings.[
      {key: "gg", command: Commands.gg.id, condition: commandCondition},
      {key: "<S-G>", command: Commands.g.id, condition: commandCondition},
      {key: "j", command: Commands.j.id, condition: commandCondition},
      {key: "k", command: Commands.k.id, condition: commandCondition},
      {key: "<DOWN>", command: Commands.j.id, condition: commandCondition},
      {key: "<UP>", command: Commands.k.id, condition: commandCondition},
      {key: "<CR>", command: Commands.enter.id, condition: commandCondition},
      {key: "zz", command: Commands.zz.id, condition: commandCondition},
      {key: "zb", command: Commands.zb.id, condition: commandCondition},
      {key: "zt", command: Commands.zt.id, condition: commandCondition},
      {key: "0", command: Commands.digit0.id, condition: commandCondition},
      {key: "1", command: Commands.digit1.id, condition: commandCondition},
      {key: "2", command: Commands.digit2.id, condition: commandCondition},
      {key: "3", command: Commands.digit3.id, condition: commandCondition},
      {key: "4", command: Commands.digit4.id, condition: commandCondition},
      {key: "5", command: Commands.digit5.id, condition: commandCondition},
      {key: "6", command: Commands.digit6.id, condition: commandCondition},
      {key: "7", command: Commands.digit7.id, condition: commandCondition},
      {key: "8", command: Commands.digit8.id, condition: commandCondition},
      {key: "9", command: Commands.digit9.id, condition: commandCondition},
    ];
};

module Contributions = {
  let contextKeys = ContextKeys.[vimListNavigation];

  let keybindings = Keybindings.keybindings;

  let commands =
    Commands.[
      gg,
      g,
      j,
      k,
      enter,
      zz,
      zb,
      zt,
      digit0,
      digit1,
      digit2,
      digit3,
      digit4,
      digit5,
      digit6,
      digit7,
      digit8,
      digit9,
    ];
};

// VIEW

module View = {
  open Revery.UI;
  open Revery.UI.Components;

  module Colors = Feature_Theme.Colors;

  module Constants = {
    let scrollWheelMultiplier = 25;
    let additionalRowsToRender = 1;
    let scrollBarThickness = 6;
    let minimumThumbSize = 4;
  };

  module Animation = {
    let scrollSpring =
      Spring.Options.create(~stiffness=310., ~damping=30., ());
  };

  module Styles = {
    open Style;

    let container = (~height) => [
      position(`Relative),
      top(0),
      left(0),
      switch (height) {
      | Some(height) => Style.height(height)
      | None => flexGrow(1)
      },
      overflow(`Hidden),
    ];

    let slider = [
      position(`Absolute),
      right(0),
      top(0),
      bottom(0),
      width(Constants.scrollBarThickness),
    ];

    let viewport = (~isScrollbarVisible) => [
      position(`Absolute),
      top(0),
      left(0),
      bottom(0),
      right(isScrollbarVisible ? 0 : Constants.scrollBarThickness),
    ];

    let item = (~offset, ~rowHeight, ~bg) => [
      position(`Absolute),
      backgroundColor(bg),
      top(offset),
      left(0),
      right(0),
      height(rowHeight),
    ];
  };
  let renderHelper =
      (
        ~items,
        ~focusIndex,
        ~hovered,
        ~selected,
        ~hoverBg,
        ~focusBg,
        ~selectedBg,
        ~onMouseClick,
        ~onMouseOver,
        ~onMouseOut,
        ~viewportWidth,
        ~viewportHeight,
        ~rowHeight,
        ~count,
        ~scrollTop,
        ~render,
      ) =>
    if (rowHeight <= 0) {
      [];
    } else {
      let startRow = scrollTop / rowHeight;
      let startY = scrollTop mod rowHeight;
      let rowsToRender =
        viewportHeight
        / rowHeight
        + Constants.additionalRowsToRender
        |> IntEx.clamp(~lo=0, ~hi=count - startRow);
      let indicesToRender = List.init(rowsToRender, i => i + startRow);

      let itemView = i => {
        let rowY = (i - startRow) * rowHeight;
        let offset = rowY - startY;
        let isHovered = hovered == Some(i);
        let isSelected = selected == i;

        let bg =
          if (isSelected) {
            selectedBg;
          } else if (isHovered) {
            hoverBg;
          } else if (focusIndex == Some(i)) {
            focusBg;
          } else {
            Revery.Colors.transparentWhite;
          };

        <Clickable
          onMouseEnter={_ => onMouseOver(i)}
          onMouseLeave={_ => onMouseOut(i)}
          onClick={_ => onMouseClick(i)}
          style={Styles.item(~offset, ~rowHeight, ~bg)}>
          {render(
             ~availableWidth=viewportWidth,
             ~index=i,
             ~hovered=hovered == Some(i),
             ~selected=selected == i,
             items[i],
           )}
        </Clickable>;
      };

      indicesToRender |> List.map(itemView) |> List.rev;
    };
  let component = React.Expert.component("Component_VimList");
  let make:
    (
      ~isActive: bool,
      ~focusedIndex: option(int),
      ~theme: ColorTheme.Colors.t,
      ~model: model('item),
      ~dispatch: msg => unit,
      ~render: (
                 ~availableWidth: int,
                 ~index: int,
                 ~hovered: bool,
                 ~selected: bool,
                 'item
               ) =>
               Revery.UI.element,
      unit
    ) =>
    _ =
    (~isActive, ~focusedIndex, ~theme, ~model, ~dispatch, ~render, ()) => {
      component(hooks => {
        let {rowHeight, viewportWidth, viewportHeight, _} = model;

        let count = Array.length(model.items);
        let contentHeight = count * rowHeight;

        let isScrollAnimated = isScrollAnimated(model);

        let ((scrollY, _setScrollYImmediately), hooks) =
          Hooks.spring(
            ~target=model.scrollY,
            ~restThreshold=10.,
            ~enabled=isScrollAnimated,
            Animation.scrollSpring,
            hooks,
          );
        let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
          let delta =
            wheelEvent.deltaY *. float(- Constants.scrollWheelMultiplier);

          dispatch(MouseWheelScrolled({delta: delta}));
        };

        let onMouseOver = idx => {
          dispatch(MouseOver({index: idx}));
        };

        let onMouseOut = idx => {
          dispatch(MouseOut({index: idx}));
        };

        let onMouseClick = idx => {
          dispatch(MouseClicked({index: idx}));
        };

        let scrollbar = {
          let maxHeight = count * rowHeight - viewportHeight;
          let thumbHeight =
            viewportHeight
            * viewportHeight
            / max(1, count * rowHeight)
            |> max(Constants.minimumThumbSize);
          let isVisible = maxHeight > 0;

          if (isVisible) {
            <View style=Styles.slider>
              <Slider
                onValueChanged={v => dispatch(ScrollbarMoved({scrollY: v}))}
                minimumValue=0.
                maximumValue={float_of_int(maxHeight)}
                sliderLength=viewportHeight
                thumbLength=thumbHeight
                value=scrollY
                trackThickness=Constants.scrollBarThickness
                thumbThickness=Constants.scrollBarThickness
                minimumTrackColor=Revery.Colors.transparentBlack
                maximumTrackColor=Revery.Colors.transparentBlack
                thumbColor={Colors.ScrollbarSlider.background.from(theme)}
                vertical=true
              />
            </View>;
          } else {
            React.empty;
          };
        };
        let hoverBg = Colors.List.hoverBackground.from(theme);
        let selectedBg =
          isActive
            ? Colors.List.activeSelectionBackground.from(theme)
            : Colors.List.inactiveSelectionBackground.from(theme);
        let focusBg =
          isActive
            ? Colors.List.focusBackground.from(theme)
            : Colors.List.inactiveFocusBackground.from(theme);
        let items =
          renderHelper(
            ~focusIndex=focusedIndex,
            ~selected=model.selected,
            ~hovered=model.hovered,
            ~items=model.items,
            ~hoverBg,
            ~selectedBg,
            ~focusBg,
            ~onMouseOver,
            ~onMouseOut,
            ~onMouseClick,
            ~viewportWidth,
            ~viewportHeight,
            ~rowHeight,
            ~count,
            ~scrollTop=int_of_float(scrollY),
            ~render,
          )
          |> React.listToElement;

        let topShadow =
          model |> showTopScrollShadow
            ? <Oni_Components.ScrollShadow.Top /> : React.empty;

        let bottomShadow =
          model |> showBottomScrollShadow
            ? <Oni_Components.ScrollShadow.Bottom /> : React.empty;

        (
          <View
            style=Style.[flexGrow(1), position(`Relative)]
            onDimensionsChanged={({height, width}) => {
              dispatch(
                ViewDimensionsChanged({
                  widthInPixels: width,
                  heightInPixels: height,
                }),
              )
            }}>
            <View
              style={Styles.container(
                // Set the height only to force it smaller, not bigger
                ~height=
                  contentHeight < viewportHeight ? Some(contentHeight) : None,
              )}
              onMouseWheel=scroll>
              <View
                style={Styles.viewport(
                  ~isScrollbarVisible=scrollbar == React.empty,
                )}>
                items
              </View>
              topShadow
              bottomShadow
              scrollbar
            </View>
          </View>,
          hooks,
        );
      });
    };
};
