open Oni_Core;
open Utility;

// MODEL

[@deriving show]
type model('item) = {
  scrollY: float,
  rowHeight: int,
  items: array('item),
  focused: option(int),
  initialRowsToRender: int,
};

let create = (~rowHeight) => {
  scrollY: 0.,
  rowHeight,
  items: [||],
  focused: None,
  initialRowsToRender: 10,
};

[@deriving show]
type command =
  | CursorToTop //gg
  | CursorToBottom // G
  | CursorDown // j
  | CursorUp; // k
//  | ScrollCursorCenter // zz
//  | ScrollCursorBottom // zb
//  | ScrollCursorTop; // zt

[@deriving show]
type msg =
  | Command(command)
  | MouseWheelScrolled({delta: float})
  | ScrollbarMoved({scrollY: float});

type outmsg =
  | Nothing;

let set = (items, model) => {...model, items};

// UPDATE

let setScrollY = (~scrollY, model) => {
  // Allow for overscroll such that the very last item is visible
  let countMinusOne = max(0, Array.length(model.items) - 1);
  let maxScroll = float(countMinusOne * model.rowHeight)
  let minScroll = 0.;

  let newScrollY = FloatEx.clamp(scrollY, ~hi=maxScroll, ~lo=minScroll);
  {...model, scrollY: newScrollY}
}

let update = (msg, model) => {
  switch (msg) {
  | Command(_) => (model, Nothing)
  | MouseWheelScrolled({ delta }) => (model |> setScrollY(~scrollY=model.scrollY +. delta), Nothing)
  | ScrollbarMoved({ scrollY }) => (model |> setScrollY(~scrollY), Nothing)
  }
};

// CONTRIBUTIONS

module Commands = {
  open Feature_Commands.Schema;

  let gg = define("vim.list.gg", Command(CursorToTop));
  let g = define("vim.list.G", Command(CursorToBottom));
  let j = define("vim.list.j", Command(CursorDown));
  let k = define("vim.list.k", Command(CursorUp));
  //  let zz = define("vim.list.zz", Command(ScrollCursorCenter));
  //  let zb = define("vim.list.zb", Command(ScrollCursorBottom));
  //  let zt = define("vim.list.zt", Command(ScrollCursorTop));
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
      {key: "G", command: Commands.g.id, condition: commandCondition},
      {key: "j", command: Commands.j.id, condition: commandCondition},
      {key: "k", command: Commands.k.id, condition: commandCondition},
    ];
  //      {
  //        key: "zz",
  //        command: Commands.zz.id,
  //        condition: commandCondition,
  //      },
  //      {
  //        key: "zb",
  //        command: Commands.zb.id,
  //        condition: commandCondition,
  //      },
  //      {
  //        key: "zt",
  //        command: Commands.zt.id,
  //        condition: commandCondition,
  //      },
};

module Contributions = {
  let contextKeys = ContextKeys.[vimListNavigation];

  let keybindings = Keybindings.keybindings;

  let commands = Commands.[gg, g, j, k];
  //    zz,
  //    zb,
  //    zt
};

// VIEW

module View = {
  open Revery;
  open Revery.UI;
  open Revery.UI.Components;

  module Colors = Feature_Theme.Colors;

  module Constants = {
    let scrollWheelMultiplier = 25;
    let additionalRowsToRender = 1;
    let scrollBarThickness = 6;
    let minimumThumbSize = 4;
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

    let item = (~offset, ~rowHeight) => [
      position(`Absolute),
      top(offset),
      left(0),
      right(0),
      height(rowHeight),
    ];
  };
  let renderHelper =
      (
        ~items,
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

        <View style={Styles.item(~offset, ~rowHeight)}>
          {render(
             ~availableWidth=viewportWidth,
             ~index=i,
             ~focused=false,
             items[i],
           )}
        </View>;
      };

      indicesToRender |> List.map(itemView) |> List.rev;
    };
  let component = React.Expert.component("Component_VimList");
  let make:
    (
      ~theme: ColorTheme.Colors.t,
      ~model: model('item),
      //    ~uniqueId: 'item => 'key,
      //    ~focused: 'key,
      //~searchText: option('item => string),
      //    ~scrollY: float,
      ~dispatch: msg => unit,
      //    ~rowHeight: float,
      ~render: (~availableWidth: int, ~index: int, ~focused: bool, 'item) =>
               Revery.UI.element,
      unit
    ) =>
    _ =
    (~theme, ~model, ~dispatch, ~render, ()) => {
      component(hooks => {
        let rowHeight = model.rowHeight;
        let (
          ((viewportWidth, viewportHeight), setViewportDimensions),
          hooks,
        ) =
          Hooks.state((100, rowHeight * model.initialRowsToRender), hooks);

        let count = Array.length(model.items);
        let contentHeight = count * model.rowHeight;

        let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
          let delta =
            wheelEvent.deltaY *. float(- Constants.scrollWheelMultiplier);

          dispatch(MouseWheelScrolled({delta: delta}));
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
                value={model.scrollY}
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
        let items =
          renderHelper(
            ~items=model.items,
            ~viewportWidth,
            ~viewportHeight,
            ~rowHeight,
            ~count,
            ~scrollTop=int_of_float(model.scrollY),
            ~render,
          )
          |> React.listToElement;

        (
          <View
            style=Style.[flexGrow(1)]
            onDimensionsChanged={({height, width}) => {
              setViewportDimensions(_ => (width, height))
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
              scrollbar
            </View>
          </View>,
          hooks,
        );
      });
    };
};
