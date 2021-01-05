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
  fontWeight: [@opaque] Weight.t,
  fontSize: float,
  font: [@opaque] Revery.Font.t,
  spaceWidth: float,
  underscoreWidth: float,
  avgCharWidth: float,
  maxCharWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: [@opaque] Smoothing.t,
};

let paint = Skia.Paint.make();

let isSameFamily = (tf1, tf2) =>
  String.equal(
    tf1 |> Revery_Font.getSkiaTypeface |> Skia.Typeface.getFamilyName,
    tf2 |> Revery_Font.getSkiaTypeface |> Skia.Typeface.getFamilyName,
  );

let loadAndValidateEditorFont =
    (
      ~requestId: int,
      ~smoothing: Smoothing.t,
      ~family: Family.t,
      ~weight: Weight.t,
      ~fontCache: FontResolutionCache.t,
      fontSize: float,
    ) => {
  let result =
    family |> Revery_Font.Family.toSkia(weight) |> Revery.Font.FontCache.load;

  Result.bind(
    result,
    font => {
      let spaceWidth =
        Revery.Font.FontRenderer.measure(~smoothing, font, fontSize, " ").
          width;
      let underscoreWidth =
        Revery.Font.FontRenderer.measure(~smoothing, font, fontSize, "_").
          width;
      let {lineHeight, descent, avgCharWidth, maxCharWidth, _}: Revery.Font.FontMetrics.t =
        Revery.Font.getMetrics(font, fontSize);
      FontResolutionCache.add((family, weight, false), font, fontCache);

      let maybeItalicFont =
        family
        |> Revery_Font.Family.toSkia(~italic=true, weight)
        |> Revery.Font.FontCache.load;

      switch (maybeItalicFont) {
      | Ok(italicFont) =>
        if (isSameFamily(font, italicFont)) {
          FontResolutionCache.add(
            (family, weight, true),
            italicFont,
            fontCache,
          );
        }
      | Error(msg) =>
        Log.warnf(m =>
          m(
            "Unable to load italic variant of %s with weight %s: %s",
            font |> getSkiaTypeface |> Skia.Typeface.getFamilyName,
            Revery.Font.Weight.show(weight),
            msg,
          )
        )
      };
      let bolderWeight = Oni_Core.Font.bolder(weight);
      let maybeBoldFont =
        family
        |> Revery_Font.Family.toSkia(bolderWeight)
        |> Revery.Font.FontCache.load;

      switch (maybeBoldFont) {
      | Ok(boldFont) =>
        if (isSameFamily(font, boldFont)) {
          FontResolutionCache.add(
            (family, bolderWeight, false),
            boldFont,
            fontCache,
          );
        }
      | Error(msg) =>
        Log.warnf(m =>
          m(
            "Unable to load bold variant of %s with weight %s: %s",
            font |> getSkiaTypeface |> Skia.Typeface.getFamilyName,
            Revery.Font.Weight.show(bolderWeight),
            msg,
          )
        )
      };

      FontResolutionCache.trim(fontCache);

      Ok((
        requestId,
        {
          fontFamily: family,
          fontWeight: weight,
          fontSize,
          font,
          spaceWidth,
          underscoreWidth,
          avgCharWidth,
          maxCharWidth,
          measuredHeight: lineHeight,
          descenderHeight: descent,
          smoothing,
        },
      ));
    },
  );
};
