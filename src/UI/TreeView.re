open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2.UI.TreeView"));

module type TreeModel = {
  type t;

  let children: t => list(t);
  let kind: t => [ | `Node([ | `Open | `Closed]) | `Leaf];
  let expandedSubtreeSize: t => int;
};

module Constants = {
  let arrowSize = 15.;
  let arrowSizeI = 15;
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

  let item = (~itemHeight) => [
    height(itemHeight),
    cursor(Revery.MouseCursors.pointer),
    flexDirection(`Row),
    overflow(`Hidden),
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
      style=Style.[width(Constants.arrowSizeI), height(Constants.arrowSizeI)]
    />;

  let rec nodeView =
          (
            ~renderContent,
            ~itemHeight,
            ~clipRange as (clipStart, clipEnd),
            ~onClick,
            ~node,
            (),
          ) => {
    let subtreeSize = Model.expandedSubtreeSize(node);

    let placeholder = (~size, ()) =>
      <View style={Styles.placeholder(~height=size * itemHeight)} />;

    let item = (~arrow, ()) =>
      <Sneakable
        onClick={() => onClick(node)} style={Styles.item(~itemHeight)}>
        <arrow />
        {renderContent(node)}
      </Sneakable>;

    let renderChildren = children => {
      let rec loop = (count, elements, children) =>
        switch (children) {
        | [child, ...rest] =>
          let element =
            <nodeView
              renderContent
              itemHeight
              clipRange=(clipStart - count, clipEnd - count)
              onClick
              node=child
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

  let useScroll = (~itemHeight, ~count, ~viewportHeight, ~scrollOffset) => {
    // We need to keep the previous value to know which edge to align a revealed item to
    let%hook (prevScrollTop, setPrevScrollTop) = Hooks.ref(0);
    // The internal value is used if scrollOffset isn't being passed in
    let%hook (internalScrollTop, setInternalScrollTop) = Hooks.state(0);

    let targetScrollTop =
      scrollOffset
      |> Option.map(
           fun
           | `Start(offset) => int_of_float(offset *. float(itemHeight))
           | `Middle(offset) => {
               let pixelOffset = int_of_float(offset *. float(itemHeight));
               let halfHeight = (viewportHeight - itemHeight) / 2;
               pixelOffset - halfHeight;
             }
           | `Reveal(index) => {
               let offset = index * itemHeight;
               if (offset < prevScrollTop) {
                 // out of view above, so align with top edge
                 offset;
               } else if (offset + itemHeight > prevScrollTop + viewportHeight) {
                 // out of view below, so align with bottom edge
                 offset + itemHeight - viewportHeight;
               } else {
                 prevScrollTop;
               };
             },
         )
      |> Option.value(~default=internalScrollTop)
      // Make sure we're not scrolled past the items
      |> IntEx.clamp(~lo=0, ~hi=itemHeight * count - viewportHeight);

    setPrevScrollTop(targetScrollTop);

    let%hook (actualScrollTop, setScrollTopImmediately) =
      Hooks.spring(
        ~target=float(targetScrollTop),
        ~restThreshold=3.,
        Spring.Options.stiff,
      );

    let setScrollTopImmediately = updater =>
      setInternalScrollTop(_ => {
        let scrollTop' =
          updater(targetScrollTop)
          |> IntEx.clamp(~lo=0, ~hi=itemHeight * count - viewportHeight);
        setScrollTopImmediately(float(scrollTop'));
        scrollTop';
      });

    (int_of_float(actualScrollTop), setScrollTopImmediately);
  };

  let%component make =
                (
                  ~children as renderContent,
                  ~itemHeight,
                  ~initialRowsToRender=10,
                  ~onClick,
                  ~scrollOffset:
                     option(
                       [ | `Start(float) | `Middle(float) | `Reveal(int)],
                     )=?,
                  ~onScrollOffsetChange:
                     [ | `Start(float) | `Middle(float) | `Reveal(int)] =>
                     unit=_ => (),
                  ~tree,
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

    let count = Model.expandedSubtreeSize(tree);

    let%hook (scrollTop, setScrollTop) =
      useScroll(
        ~itemHeight,
        ~count,
        ~viewportHeight=menuHeight,
        ~scrollOffset,
      );
    let setScrollTop = updater =>
      setScrollTop(scrollTop => {
        let scrollTop' = updater(scrollTop);
        onScrollOffsetChange(
          `Start(float(scrollTop') /. float(itemHeight)),
        );
        scrollTop';
      });

    let onMouseWheel = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      let delta =
        int_of_float(wheelEvent.deltaY) * (- Constants.scrollWheelMultiplier);

      setScrollTop(target => target + delta);
    };

    let maxHeight = count * itemHeight - menuHeight;
    let showScrollbar = maxHeight > 0;

    let scrollbar = () => {
      let thumbHeight = menuHeight * menuHeight / max(1, count * itemHeight);

      <View style=Styles.slider>
        <Slider
          onValueChanged={value => setScrollTop(_ => int_of_float(value))}
          minimumValue=0.
          maximumValue={float_of_int(maxHeight)}
          sliderLength=menuHeight
          thumbLength=thumbHeight
          value={float(scrollTop)}
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
      onMouseWheel>
      <View style={Styles.viewport(~showScrollbar)}>
        <View style={Styles.content(~scrollTop)}>
          <nodeView renderContent itemHeight clipRange onClick node=tree />
        </View>
      </View>
      {showScrollbar ? <scrollbar /> : React.empty}
    </View>;
  };
};
