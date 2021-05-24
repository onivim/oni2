open EditorCoreTypes;
open Revery.UI;
module Clickable = Components.Clickable;

module LineNumber = EditorCoreTypes.LineNumber;

module Styles = {
  open Style;
  let container = (~opacity as opac, ~totalHeight, ~pixelY) => {
    [
      position(`Absolute),
      top(int_of_float(pixelY)),
      left(0),
      right(0),
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

  let inner = (~opacity as opac, ~yOffset, ~xOffset) => [
    position(`Absolute),
    top(int_of_float(yOffset)),
    left(int_of_float(xOffset)),
    right(0),
    flexDirection(`Row),
    flexGrow(1),
    opacity(opac),
  ];
};

module Item = {
  let%component make =
                (
                  ~dispatch: Msg.t => unit,
                  ~inlineKey: string,
                  ~uniqueId: string,
                  ~opacity: float,
                  ~xOffset: float,
                  ~yOffset: float,
                  ~lineNumber: LineNumber.t,
                  ~children,
                  (),
                ) => {
    let%hook (measuredHeight, heightChangedDispatch) =
      Hooks.reducer(~initialState=0, (newHeight, _prev) => newHeight);

    let%hook () =
      Hooks.effect(
        If((!=), (measuredHeight, uniqueId)),
        () => {
          if (measuredHeight > 0) {
            dispatch(
              Msg.InlineElementSizeChanged({
                key: inlineKey,
                uniqueId,
                line: lineNumber,
                height: measuredHeight,
              }),
            );
          };
          None;
        },
      );
    // COMPONENT
    <View
      style={Styles.inner(~xOffset, ~yOffset, ~opacity)}
      onDimensionsChanged={({height, _}) => {heightChangedDispatch(height)}}>
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
                 inlineKey
                 uniqueId
                 dispatch
                 lineNumber=line
                 xOffset={gutterWidth +. leadingWhitespacePixels}
                 yOffset=height
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

    <View
      style={Styles.container(
        ~opacity=maxOpacity,
        ~totalHeight=int_of_float(totalHeight),
        ~pixelY=pixelY -. totalHeight,
      )}>
      {elems |> React.listToElement}
      shadow
    </View>;
  };
};
