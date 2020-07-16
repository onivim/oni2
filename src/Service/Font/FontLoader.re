/*
 * FontLoader.re
 *
 * This implements an updater (reducer + side effects) for managing fonts
 */

open Oni_Core;
open Revery.Font;

module Log = (val Log.withNamespace("Oni2.Service.FontLoader"));

[@deriving show({with_path: false})]
type t = {
  fontFamily: [@opaque] Family.t,
  fontSize: float,
  font: [@opaque] Revery.Font.t,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: [@opaque] Smoothing.t,
};

/* let loadAndValidateEditorFont =
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
             let (isMono, c1width, _) =
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
                       ~mono=true,
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
                       ~mono=true,
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
   }; */

let paint = Skia.Paint.make();

let isFontMonospace = font => {
  let metrics = Skia.FontMetrics.make();
  Skia.Paint.setTypeface(paint, font |> Revery_Font.getSkiaTypeface);
  Skia.Paint.getFontMetrics(paint, metrics, 1.0) |> (ignore: float => unit);
  let avgCharWidth = Skia.FontMetrics.getAvgCharacterWidth(metrics);
  let maxCharWidth = Skia.FontMetrics.getMaxCharacterWidth(metrics);

  maxCharWidth -. avgCharWidth < 0.1;
};

let loadAndValidateEditorFont =
    (
      ~requestId: int,
      ~smoothing: Smoothing.t,
      ~family: Family.t,
      ~fontCache: FontResolutionCache.t,
      fontSize: float,
    ) => {
  let result =
    family
    |> Revery_Font.Family.toSkia(Weight.Normal)
    |> Revery.Font.FontCache.load;

  Result.bind(
    result,
    font => {
      let isMono = isFontMonospace(font);
      if (!isMono) {
        Error("Not a monospace font.");
      } else {
        let {lineHeight, descent, _}: Revery.Font.FontMetrics.t =
          Revery.Font.getMetrics(font, fontSize);
        FontResolutionCache.add(
          (family, Weight.Normal, false),
          font,
          fontCache,
        );

        let maybeItalicFont =
          family
          |> Revery_Font.Family.toSkia(~italic=true, Weight.Normal)
          |> Revery.Font.FontCache.load;

        switch (maybeItalicFont) {
        | Ok(italicFont) =>
          let isMono = isFontMonospace(italicFont);
          if (isMono) {
            FontResolutionCache.add(
              (family, Weight.Normal, true),
              italicFont,
              fontCache,
            );
          };
        | Error(msg) =>
          Log.warnf(m =>
            m(
              "Unable to load italic variant of %s: %s",
              font |> getSkiaTypeface |> Skia.Typeface.getFamilyName,
              msg,
            )
          )
        };

        let maybeBoldFont =
          family
          |> Revery_Font.Family.toSkia(Weight.Bold)
          |> Revery.Font.FontCache.load;

        switch (maybeBoldFont) {
        | Ok(boldFont) =>
          let isMono = isFontMonospace(boldFont);
          if (isMono) {
            FontResolutionCache.add(
              (family, Weight.Bold, false),
              boldFont,
              fontCache,
            );
          };
        | Error(msg) =>
          Log.warnf(m =>
            m(
              "Unable to load bold variant of %s: %s",
              font |> getSkiaTypeface |> Skia.Typeface.getFamilyName,
              msg,
            )
          )
        };

        FontResolutionCache.trim(fontCache);

        let FontRenderer.{width: measuredWidth, _} =
          Revery.Font.FontRenderer.measure(~smoothing, font, fontSize, "x");
        Ok((
          requestId,
          {
            fontFamily: family,
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
  );
};
