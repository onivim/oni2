open EditorCoreTypes;
open Revery.UI;
module Clickable = Components.Clickable;

module LineNumber = EditorCoreTypes.LineNumber;

module Styles = {
  open Style;
  let container = (~minimapWidth, ~opacity as opac, ~totalHeight, ~pixelY) => {
    [
      position(`Absolute),
      top(int_of_float(pixelY)),
      left(0),
      right(int_of_float(minimapWidth)),
      height(totalHeight),
      opacity(opac),
      pointerEvents(`Allow),
    ];
  };

  let shadowContainer = (~height as h) => {
    [
      position(`Absolute),
      left(0),
      right(0),
      bottom(0),
      height(h),
      opacity(0.8),
      pointerEvents(`Ignore),
    ];
  };

  let inner = (~availableWidth, ~opacity as opac, ~yOffset, ~xOffset) => [
    position(`Absolute),
    top(int_of_float(yOffset)),
    left(int_of_float(xOffset)),
    width(availableWidth),
    flexDirection(`Row),
    flexGrow(1),
    opacity(opac),
  ];
};

module Item = {
  let make =
      (
        ~children,
        ~opacity: float,
        ~xOffset: float,
        ~yOffset: float,
        ~availableWidth: int,
        (),
      ) => {
    // COMPONENT
    <View style={Styles.inner(~availableWidth, ~xOffset, ~yOffset, ~opacity)}>
      children
    </View>;
  };
};

module Container = {
  let make =
      (
        ~gutterWidth,
        ~config,
        ~editor,
        ~isVisible,
        ~line,
        ~dispatch,
        ~theme,
        ~uiFont,
        (),
      ) => {
    let inlineElements = Editor.getInlineElements(~line, editor);

    let leadingWhitespacePixels =
      Editor.getLeadingWhitespacePixels(line, editor);

    let minimapWidth = Editor.minimapWidthInPixels(editor);

    let (maxOpacity, totalHeight, elems) =
      inlineElements
      |> List.fold_left(
           (acc, inlineElement: InlineElements.element) => {
             let (currentOpacity, height, accElements) = acc;
             let h = height;
             let uniqueId = inlineElement.uniqueId;
             let elem = inlineElement.view(~theme, ~uiFont);
             let inlineKey = inlineElement.key;
             let opacity = inlineElement.opacity |> Component_Animation.get;
             let currentHeight =
               Component_Animation.get(inlineElement.height);
             let clickable =
               <Clickable
                 style=Style.[
                   position(`Absolute),
                   top(int_of_float(h)),
                   left(0),
                   right(0),
                   height(int_of_float(currentHeight)),
                   pointerEvents(`Allow),
                 ]
                 onClick={_ =>
                   dispatch(
                     Msg.InlineElementClicked({
                       key: inlineKey,
                       uniqueId,
                       command: inlineElement.command,
                     }),
                   )
                 }>
                 React.empty
               </Clickable>;

             let newElement =
               <Item
                 xOffset={gutterWidth +. leadingWhitespacePixels}
                 yOffset=height
                 availableWidth={inlineElement.width}
                 opacity>
                 <elem />
               </Item>;

             (
               max(currentOpacity, opacity),
               height +. currentHeight,
               [newElement, clickable, ...accElements],
             );
           },
           (0., 0., []),
         );

    let lineNumber = line;
    let ({y: pixelY, _}: PixelPosition.t, _width) =
      Editor.bufferBytePositionToPixel(
        ~position=BytePosition.{line: lineNumber, byte: ByteIndex.ofInt(0)},
        editor,
      );

    // Rendering shadows can be expensive, so let's not do it if
    // we don't need to...
    let shadow =
      isVisible
      && Feature_Configuration.GlobalConfiguration.shadows.get(config)
        ? <View
            style={Styles.shadowContainer(~height=int_of_float(totalHeight))}>
            <Oni_Components.ScrollShadow.Top
              opacity={maxOpacity *. 1.0}
              height=5
              theme
            />
            <Oni_Components.ScrollShadow.Bottom
              opacity={maxOpacity *. 1.0}
              height=5
              theme
            />
          </View>
        : React.empty;
    // Forward mouse-wheel events to editor
    let onMouseWheel = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
      dispatch(
        Msg.EditorMouseWheel({
          deltaY: wheelEvent.deltaY *. (-1.),
          deltaX: wheelEvent.deltaX,
          shiftKey: wheelEvent.shiftKey,
        }),
      );

    <View
      style={Styles.container(
        ~minimapWidth,
        ~opacity=maxOpacity,
        ~totalHeight=int_of_float(totalHeight),
        ~pixelY=pixelY -. totalHeight,
      )}
      onMouseWheel>
      {elems |> React.listToElement}
      shadow
    </View>;
  };
};
