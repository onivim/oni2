open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
open Oni_Model;

module type TreeModel = {
  type t;

  let children: t => list(t);
  let kind: t => [ | `Node([ | `Open | `Closed]) | `Leaf];
  let expandedSubtreeSize: t => int;
};

module Constants = {
  let arrowSize = 15;
  let indentSize = 12.;
  let scrollWheelMultiplier = 25;
  let scrollBarThickness = 6;
  let scrollTrackColor = Color.rgba(0.0, 0.0, 0.0, 0.4);
  let scrollThumbColor = Color.rgba(0.5, 0.5, 0.5, 0.4);
};

module Styles = {
  open Style;

  let container = [overflow(`Hidden), flexGrow(1)];

  let slider = [
    position(`Absolute),
    right(0),
    top(0),
    bottom(0),
    width(Constants.scrollBarThickness),
  ];

  let viewport = (~showScrollbar) => [
    position(`Absolute),
    top(0),
    left(0),
    bottom(0),
    right(showScrollbar ? 0 : Constants.scrollBarThickness),
  ];

  let content = (~scrollTop) => [
    position(`Absolute),
    top(- scrollTop),
    left(0),
    bottom(0),
  ];

  let item = (~itemHeight, ~isFocused, ~theme: Theme.t) => [
    height(itemHeight),
    cursor(Revery.MouseCursors.pointer),
    flexDirection(`Row),
    overflow(`Hidden),
    backgroundColor(isFocused ? theme.menuSelectionBackground : Colors.transparentWhite),
  ];

  let placeholder = (~height) => [Style.height(height)];

  let children = [transform(Transform.[TranslateX(Constants.indentSize)])];

  let loading = [
    fontFamily("selawk.ttf"),
    fontSize(12),
    color(Colors.white),
  ];
};

module Make = (Model: TreeModel) => {
  let arrow = (~isOpen, ()) =>
    <FontIcon
      fontSize=Constants.arrowSize
      color=Colors.white
      icon={isOpen ? FontAwesome.caretDown : FontAwesome.caretRight}
      backgroundColor=Colors.transparentWhite
    />;

  let noArrow = () =>
    <View
      style=Style.[width(Constants.arrowSize), height(Constants.arrowSize)]
    />;

  let rec nodeView =
          (
            ~renderContent,
            ~focus,
            ~itemHeight,
            ~clipRange as (clipStart, clipEnd),
            ~onClick,
            ~node,
            ~theme,
            (),
          ) => {
    let subtreeSize = Model.expandedSubtreeSize(node);

    let (isFocused, childFocus) =
      switch (focus) {
      | Some([last]) when last == node => (true, None)
      | Some([head, ...tail]) when head == node => (false, Some(tail))
      | Some(v) => (false, None)
      | _ => (false, None)
      };

    let placeholder = (~size, ()) =>
      <View style={Styles.placeholder(~height=size * itemHeight)} />;

    let item = (~arrow, ()) =>
      <Clickable
        onClick={() => onClick(node)}
        style={Styles.item(~itemHeight, ~isFocused, ~theme)}>
        <arrow />
        {renderContent(node)}
      </Clickable>;

    let renderChildren = children => {
      let rec loop = (count, elements, children) =>
        switch (children) {
        | [child, ...rest] =>
          let element =
            <nodeView
              renderContent
              focus=childFocus
              itemHeight
              clipRange=(clipStart - count, clipEnd - count)
              onClick
              node=child
              theme
            />;

          loop(
            count + Model.expandedSubtreeSize(child),
            [element, ...elements],
            rest,
          );

        | [] => elements |> List.rev |> React.listToElement
        };

      loop(1, [], children);
    };

    if (subtreeSize < clipStart || clipEnd < 0) {
      <placeholder
        // If the entire node is out of view, render a placeholder with the appropriate height
        size=subtreeSize
      />;
    } else {
      switch (Model.kind(node)) {
      | `Node(state) =>
        <View>
          <item arrow={arrow(~isOpen=state == `Open)} />
          <View style=Styles.children>
            {switch (state) {
             | `Open => node |> Model.children |> renderChildren
             | `Closed => React.empty
             }}
          </View>
        </View>

      | `Leaf => <item arrow=noArrow />
      };
    };
  };

  let%component make =
                (
                  ~children as renderContent,
                  ~focus: option(list(Model.t)),
                  ~itemHeight,
                  ~initialRowsToRender=10,
                  ~onClick,
                  ~tree,
                  ~theme,
                  (),
                ) => {
    let%hook (outerRef, setOuterRef) = Hooks.ref(None);

    let menuHeight =
      switch (outerRef) {
      | Some(node) =>
        let dimensions: Dimensions.t = node#measurements();
        dimensions.height;
      | None => itemHeight * initialRowsToRender
      };

    let%hook (scrollTop, setScrollTop) = Hooks.state(0);

    let count = Model.expandedSubtreeSize(tree);

    // Make sure we're not scrolled past the items
    let scrollTop =
      scrollTop |> Utility.clamp(~lo=0, ~hi=itemHeight * count - menuHeight);

    let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      let delta =
        int_of_float(wheelEvent.deltaY) * (- Constants.scrollWheelMultiplier);

      setScrollTop(_ => scrollTop + delta);
    };

    let maxHeight = count * itemHeight - menuHeight;
    let showScrollbar = maxHeight > 0;

    let scrollbar = () => {
      let thumbHeight = menuHeight * menuHeight / max(1, count * itemHeight);

      <View style=Styles.slider>
        <Slider
          onValueChanged={v => setScrollTop(_ => int_of_float(v))}
          minimumValue=0.
          maximumValue={float_of_int(maxHeight)}
          sliderLength=menuHeight
          thumbLength=thumbHeight
          value={float_of_int(scrollTop)}
          trackThickness=Constants.scrollBarThickness
          thumbThickness=Constants.scrollBarThickness
          minimumTrackColor=Constants.scrollTrackColor
          maximumTrackColor=Constants.scrollTrackColor
          thumbColor=Constants.scrollThumbColor
          vertical=true
        />
      </View>;
    };

    let clipRange = (
      scrollTop / itemHeight,
      (scrollTop + menuHeight) / itemHeight,
    );

    <View
      style=Styles.container
      ref={ref => setOuterRef(Some(ref))}
      onMouseWheel=scroll>
      <View style={Styles.viewport(~showScrollbar)}>
        <View style={Styles.content(~scrollTop)}>
          <nodeView
            renderContent
            focus
            itemHeight
            clipRange
            onClick
            node=tree
            theme
          />
        </View>
      </View>
      {showScrollbar ? <scrollbar /> : React.empty}
    </View>;
  };
};
