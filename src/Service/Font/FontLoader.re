/*
 * FontLoader.re
 *
 * This implements an updater (reducer + side effects) for managing fonts
 */

open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Sservice.FontLoader"));

[@deriving show({with_path: false})]
type t = {
  fontFile: string,
  fontSize: float,
  font: [@opaque] Revery.Font.t,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: [@opaque] Revery.Font.Smoothing.t,
};

let loadAndValidateEditorFont =
    (
      ~requestId: int,
      ~smoothing: Revery.Font.Smoothing.t,
      fullPath,
      fontSize: float,
    ) => {
  Log.tracef(m =>
    m("loadAndValidateEditorFont path: %s | size: %f", fullPath, fontSize)
  );

  switch (smoothing) {
  | None => print_endline("---- NON!")
  | Antialiased => print_endline("---- ANTI!")
  | SubpixelAntialiased => print_endline("---- SUB!")
  };

  fullPath
  |> Revery.Font.FontCache.load
  |> (
    r =>
      Stdlib.Result.bind(
        r,
        font => {
          let character1 =
            Revery.Font.FontRenderer.measure(~smoothing, font, fontSize, "H");
          let character2 =
            Revery.Font.FontRenderer.measure(~smoothing, font, fontSize, "i");

          if (!Float.equal(character1.width, character2.width)) {
            Error("Not a monospace font");
          } else {
            let measuredWidth = character1.width;
            let {lineHeight, descent, _}: Revery.Font.FontMetrics.t =
              Revery.Font.getMetrics(font, fontSize);

            Log.debugf(m => m("Measured width: %f ", measuredWidth));
            Log.debugf(m => m("Line height: %f ", lineHeight));

            Ok((
              requestId,
              {
                fontFile: fullPath,
                fontSize,
                font,
                measuredWidth,
                measuredHeight: lineHeight,
                descenderHeight: descent,
                smoothing,
              },
            ));
          };
        },
      )
  );
};
