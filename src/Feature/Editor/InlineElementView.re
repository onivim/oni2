open EditorCoreTypes;
open Revery.UI;

module LineNumber = EditorCoreTypes.LineNumber;

module Styles = {
  open Style;
  let container = (~opacity as opac, ~pixelY) => {
    [
      opacity(opac),
      position(`Absolute),
      top(pixelY |> int_of_float),
      left(0),
      right(0),
    ];
  };

  let inner = [
    position(`Absolute),
    bottom(0),
    left(0),
    right(0),
    flexDirection(`Row),
    flexGrow(1),
  ];
};

module Animation = {
  let fadeIn =
    Revery.(Animation.(animate(Time.milliseconds(500)) |> tween(0., 1.0)));

  let expand =
    Revery.(Animation.(animate(Time.milliseconds(200)) |> tween(0., 1.0)));
};

let%component make =
              (
                ~dispatch: Msg.t => unit,
                ~inlineKey: string,
                ~uniqueId: string,
                ~editor,
                ~lineNumber: LineNumber.t,
                ~children,
                (),
              ) => {
  // HOOKS

  // TODO: Graceful fade-in transition
  // Ensure that existing code-lenses don't get paved
  let opacity = 1.0;

  let%hook (measuredHeight, heightChangedDispatch) =
    Hooks.reducer(~initialState=0, (newHeight, _prev) => newHeight);

  // TODO: Bring back expansion animation
  //  let%hook (animatedHeight, _heightAnimationState, _resetHeight) =
  //    Hooks.animation(
  //      ~name="Inline Element Expand",
  //      Animation.expand,
  //      ~active=true,
  //    );

  let animatedHeight = 1.0;

  let%hook () =
    Hooks.effect(
      OnMountAndIf((!=), measuredHeight),
      () => {
        if (measuredHeight > 0) {
          dispatch(
            Msg.InlineElementSizeChanged({
              key: inlineKey,
              uniqueId,
              height: int_of_float(float(measuredHeight) *. animatedHeight),
            }),
          );
        };
        None;
      },
    );

  // COMPONENT

  let ({y: pixelY, _}: PixelPosition.t, _width) =
    Editor.bufferBytePositionToPixel(
      ~position=BytePosition.{line: lineNumber, byte: ByteIndex.ofInt(0)},
      editor,
    );

  <View style={Styles.container(~opacity, ~pixelY)}>
    <View
      style=Styles.inner
      onDimensionsChanged={({height, _}) => {heightChangedDispatch(height)}}>
      children
    </View>
  </View>;
};
