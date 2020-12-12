open EditorCoreTypes;
open Oni_Core;
open Utility;
open Component_Animation;

module GlobalState = {
  let lastId = ref(0);

  let generateId = () => {
    let id = lastId^;
    incr(lastId);
    id;
  };
};

type inlineElement = {
  reconcilerKey: Brisk_reconciler.Key.t,
  // hidden: bool,
  key: string,
  uniqueId: string,
  lineNumber: EditorCoreTypes.LineNumber.t,
  view:
    (~theme: Oni_Core.ColorTheme.Colors.t, ~uiFont: UiFont.t, unit) =>
    Revery.UI.element,
};

module WrapMode = {
  [@deriving show]
  type t =
    | NoWrap
    | Viewport;
};

module WrapState = {
  [@deriving show]
  type t =
    | NoWrap({
        [@opaque]
        wrapping: Wrapping.t,
      })
    | Viewport({
        lastWrapPixels: float,
        [@opaque]
        wrapping: Wrapping.t,
      });

  let make = (~pixelWidth: float, ~wrapMode: WrapMode.t, ~buffer) => {
    switch (wrapMode) {
    | NoWrap =>
      NoWrap({wrapping: Wrapping.make(~wrap=WordWrap.none, ~buffer)})
    | Viewport =>
      Viewport({
        lastWrapPixels: pixelWidth,
        wrapping:
          Wrapping.make(~wrap=WordWrap.fixed(~pixels=pixelWidth), ~buffer),
      })
    };
  };

  let wrapping =
    fun
    | NoWrap({wrapping}) => wrapping
    | Viewport({wrapping, _}) => wrapping;

  let resize = (~pixelWidth: float, ~buffer, wrapState) => {
    switch (wrapState) {
    // All the cases where we don't need to update wrapping...
    | NoWrap(_) as nowrap => nowrap
    | Viewport({lastWrapPixels, _}) when lastWrapPixels != pixelWidth =>
      let wrapping =
        Wrapping.make(~wrap=WordWrap.fixed(~pixels=pixelWidth), ~buffer);
      Viewport({lastWrapPixels: pixelWidth, wrapping});
    | Viewport(_) as viewport => viewport
    };
  };

  let map = f =>
    fun
    | NoWrap({wrapping}) => NoWrap({wrapping: f(wrapping)})
    | Viewport({wrapping, lastWrapPixels}) =>
      Viewport({wrapping: f(wrapping), lastWrapPixels});

  let update = (~update, ~buffer, wrapState) => {
    wrapState |> map(Wrapping.update(~update, ~newBuffer=buffer));
  };
};
