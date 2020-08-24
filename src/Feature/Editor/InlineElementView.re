open EditorCoreTypes;
open Revery.UI;

module LineNumber = EditorCoreTypes.LineNumber;

module Styles = {
  open Style;
  let container = (~pixelY) => {
    [position(`Absolute), top(pixelY |> int_of_float), left(0), right(0)];
  };

  let inner = [
    position(`Absolute),
    bottom(0),
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
                ~uniqueId: string,
                ~editor,
                ~lineNumber: LineNumber.t,
                ~children,
                (),
              ) => {
  // HOOKS
  let%hook (opacity, _animationState, _reset) =
    Hooks.animation(Animation.fadeIn, ~active=true);

  let%hook (measuredHeight, heightChangedDispatch) =
    Hooks.reducer(~initialState=0, (newHeight, _prev) => newHeight);

  let%hook (animatedHeight, _heightAnimationState, _resetHeight) =
    Hooks.animation(Animation.expand, ~active=true);

  let%hook () =
    Hooks.effect(
      OnMountAndIf((!=), animatedHeight),
      () => {
        dispatch(
          Msg.InlineElementUpdated({
            uniqueId,
            lineNumber,
            height: int_of_float(float(measuredHeight) *. animatedHeight),
          }),
        );
        None;
      },
    );

  let%hook () =
    Hooks.effect(OnMount, () => {
      Some(() => {dispatch(Msg.InlineElementRemoved({uniqueId: uniqueId}))})
    });

  // COMPONENT

  let ({pixelY, _}: Editor.pixelPosition, _width) =
    Editor.bufferBytePositionToPixel(
      ~position=BytePosition.{line: lineNumber, byte: ByteIndex.ofInt(0)},
      editor,
    );

  <View style={Styles.container(~pixelY)}>
    <View
      style=Styles.inner
      onDimensionsChanged={({height, _}) => {heightChangedDispatch(height)}}>
      <Opacity opacity> children </Opacity>
    </View>
  </View>;
};
