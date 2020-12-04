open EditorCoreTypes;
open Revery.UI;

module LineNumber = EditorCoreTypes.LineNumber;

module Styles = {
  open Style;
  let container = (~opacity as opac, ~pixelY) => {
    [
      position(`Absolute),
      top(int_of_float(pixelY)),
      left(0),
      right(0),
      opacity(opac),
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
    ];
  };

  let inner = (~opacity as opac, ~yOffset) => [
    position(`Absolute),
    bottom(int_of_float(yOffset)),
    left(0),
    right(0),
    flexDirection(`Row),
    flexGrow(1),
    opacity(opac),
  ];
};

module Animation = {
  let fadeIn =
    Revery.(Animation.(animate(Time.milliseconds(500)) |> tween(0., 1.0)));

  let expand =
    Revery.(Animation.(animate(Time.milliseconds(200)) |> tween(0., 1.0)));
};

module Item = {
  let%component make =
                (
                  ~dispatch: Msg.t => unit,
                  ~inlineKey: string,
                  ~uniqueId: string,
                  ~opacity: float,
                  ~editor,
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
      style={Styles.inner(~yOffset, ~opacity)}
      onDimensionsChanged={({height, _}) => {heightChangedDispatch(height)}}>
      children
    </View>;
  };
};

module Container = {
  let make = (~editor, ~line, ~dispatch, ~theme, ~uiFont, ()) => {
    let inlineElements = Editor.getInlineElements(~line, editor);

    let (maxOpacity, totalHeight, elems) =
      inlineElements
      |> List.fold_left(
           (acc, inlineElement: InlineElements.element) => {
             let (currentOpacity, height, accElements) = acc;
             let uniqueId = inlineElement.uniqueId;
             let elem = inlineElement.view(~theme, ~uiFont);
             let inlineKey = inlineElement.key;
             let opacity = inlineElement.opacity |> Component_Animation.get;

             let newElement =
               <Item
                 inlineKey
                 uniqueId
                 dispatch
                 lineNumber=line
                 yOffset=height
                 opacity
                 editor>
                 <elem />
               </Item>;

             (
               max(currentOpacity, opacity),
               height +. Component_Animation.get(inlineElement.height),
               [newElement, ...accElements],
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

    <View style={Styles.container(~opacity=maxOpacity, ~pixelY)}>
      {elems |> React.listToElement}
      <View
        style={Styles.shadowContainer(~height=int_of_float(totalHeight))}>
        <Oni_Components.ScrollShadow.Top opacity=maxOpacity height=5 />
        <Oni_Components.ScrollShadow.Bottom opacity=maxOpacity height=5 />
      </View>
    </View>;
  };
};
