/*
 * FontLoader.re
 *
 * This implements an updater (reducer + side effects) for managing fonts
 */

open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Service.FontLoader"));

[@deriving show({with_path: false})]
type t = {
  fontFile: string,
  fontFamily: [@opaque] Revery.Font.Family.t,
  fontSize: float,
  font: [@opaque] Revery.Font.t,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: [@opaque] Revery.Font.Smoothing.t,
};

let isMonospace = (~smoothing, ~font, ~fontSize) => {
  let character1 =
    Revery.Font.FontRenderer.measure(~smoothing, font, fontSize, "H");
  let character2 =
    Revery.Font.FontRenderer.measure(~smoothing, font, fontSize, "i");
  (
    Float.equal(character1.width, character2.width),
    character1.width,
    character2.width,
  );
};

let loadAndValidateEditorFont =
    (
      ~requestId: int,
      ~smoothing: Revery.Font.Smoothing.t,
      ~fontFamily: Revery_Font.Family.t,
      fullPath,
      fontSize: float,
    ) => {
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
          let (isMono, c1width, c2width) =
            isMonospace(~font, ~smoothing, ~fontSize);
          if (!isMono) {
            Error("Not a monospace font");
          } else {
            let measuredWidth = c1width;
            let {lineHeight, descent, _}: Revery.Font.FontMetrics.t =
              Revery.Font.getMetrics(font, fontSize);

            let boldPath =
              switch (
                Revery_Font.Family.resolve(
                  ~mono=true,
                  Revery_Font.Weight.Bold,
                  fontFamily,
                )
              ) {
              | Ok(f) =>
                let (isMono, c1w, c2w) =
                  isMonospace(~smoothing, ~fontSize, ~font=f);
                if (isMono) {
                  Revery_Font.Family.toPath(
                    Revery_Font.Weight.Bold,
                    fontFamily,
                  );
                } else {
                  Log.warnf(m =>
                    m(
                      "Unable to load monospace italic variant of %s: c1 width : %f c2 width : %f",
                      fullPath,
                      c1w,
                      c2w,
                    )
                  );
                  fullPath;
                };
              | Error(_) => fullPath
              };

            let italicPath =
              switch (
                Revery_Font.Family.resolve(
                  ~mono=true,
                  ~italic=true,
                  Revery_Font.Weight.Normal,
                  fontFamily,
                )
              ) {
              | Ok(f) =>
                let (isMono, c1w, c2w) =
                  isMonospace(~smoothing, ~fontSize, ~font=f);
                if (isMono) {
                  Revery_Font.Family.toPath(
                    ~italic=true,
                    Revery_Font.Weight.Normal,
                    fontFamily,
                  );
                } else {
                  Log.warnf(m =>
                    m(
                      "Unable to load monospace italic variant of %s: c1 width : %f c2 width : %f",
                      fullPath,
                      c1w,
                      c2w,
                    )
                  );
                  fullPath;
                };
              | Error(_) => fullPath
              };

            Log.debugf(m => m("Measured width: %f ", measuredWidth));
            Log.debugf(m => m("Line height: %f ", lineHeight));
            let fontFamily =
              Revery_Font.Family.fromFiles((~weight, ~italic, ~mono as _) =>
                switch (italic, weight) {
                | (true, _) => italicPath
                | (_, Revery.Font.Weight.Bold) => boldPath
                | _ => fullPath
                }
              );
            Ok((
              requestId,
              {
                fontFile: fullPath,
                fontFamily,
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
