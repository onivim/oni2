/*
 * FontLoader.re
 *
 * This implements an updater (reducer + side effects) for managing fonts
 */

open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Store.FontLoader"));

let loadAndValidateEditorFont = (~requestId: int, fullPath, fontSize: float) => {
  Log.tracef(m =>
    m("loadAndValidateEditorFont path: %s | size: %f", fullPath, fontSize)
  );

  fullPath
  |> Revery.Font.FontCache.load
  |> (
    r =>
      Stdlib.Result.bind(
        r,
        font => {
          let character1 =
            Revery.Font.FontRenderer.measure(
              ~smoothing=Revery.Font.Smoothing.default,
              font,
              fontSize,
              "H",
            );
          let character2 =
            Revery.Font.FontRenderer.measure(
              ~smoothing=Revery.Font.Smoothing.default,
              font,
              fontSize,
              "i",
            );

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
              EditorFont.create(
                ~fontFile=fullPath,
                ~fontSize,
                ~measuredWidth,
                ~measuredHeight=lineHeight,
                ~descenderHeight=descent,
                (),
              ),
            ));
          };
        },
      )
  );
};
