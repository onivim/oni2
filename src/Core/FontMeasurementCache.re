open Utility;
open Revery;

type t = {
  paint: Skia.Paint.t,
  features: list(Harfbuzz.feature),
  loadedFont: Revery.Font.FontCache.t,
  asciiTable: array(float),
  utf8Table: Hashtbl.t(int, float),
};

let create =
    (
      ~fontFamily: Revery.Font.Family.t,
      ~fontWeight: Revery.Font.Weight.t,
      ~fontSize: float,
      ~features: list(Harfbuzz.feature),
      ~smoothing: Revery.Font.Smoothing.t,
    ) => {
  let loadedFont =
    fontFamily
    |> Revery.Font.Family.toSkia(fontWeight)
    |> Revery.Font.load
    |> Result.to_option
    |> OptionEx.lazyDefault(() => {
         Revery.Font.Family.fromFile(Constants.defaultFontFile)
         |> Revery.Font.Family.toSkia(Revery.Font.Weight.Normal)
         |> Revery.Font.load
         |> Result.get_ok
       });

  let typeface = Revery.Font.getSkiaTypeface(loadedFont);

  let paint = Skia.Paint.make();
  Skia.Paint.setTextSize(paint, fontSize);
  Skia.Paint.setLcdRenderText(paint, true);
  Skia.Paint.setTypeface(paint, typeface);
  Revery.Font.Smoothing.setPaint(~smoothing, paint);

  // Pre-load the ASCII measurements - save on shaping costs
  Skia.Paint.setTextEncoding(paint, Utf8);
  let asciiTable =
    Array.init(
      256,
      idx => {
        let str = Uchar.of_int(idx) |> Zed_utf8.make(1);

        Skia.Paint.measureText(paint, str, None);
      },
    );

  let utf8Table = Hashtbl.create(256);

  Skia.Paint.setTextEncoding(paint, GlyphId);

  {
    paint,
    features,
    loadedFont,

    asciiTable,
    utf8Table,
  };
};

let measure = (uchar, cache) => {
  let code = Uchar.to_int(uchar);

  if (code < 256) {
    cache.asciiTable[code];
  } else {
    switch (Hashtbl.find_opt(cache.utf8Table, code)) {
    | Some(width) => width
    | None =>
      let glyphStrings =
        Revery.Font.shape(
          ~features=cache.features,
          cache.loadedFont,
          Zed_utf8.make(1, uchar),
        ).
          glyphStrings;

      let width =
        glyphStrings
        |> List.fold_left(
             (acc, (typeface, glyphString)) => {
               Skia.Paint.setTypeface(cache.paint, typeface);
               acc +. Skia.Paint.measureText(cache.paint, glyphString, None);
             },
             0.,
           );

      Hashtbl.add(cache.utf8Table, code, width);
      width;
    };
  };
};
