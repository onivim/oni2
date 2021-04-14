open Oni_Core;

module Zed_utf8 = ZedBundled;
module Log = (val Log.withNamespace("Oni2.Service.Font"));

[@deriving show({with_path: false})]
type font =
  Oni_Core.Font.t = {
    fontFamily: [@opaque] Revery.Font.Family.t,
    fontWeight: [@opaque] Revery.Font.Weight.t,
    fontSize: float,
    spaceWidth: float,
    underscoreWidth: float,
    avgCharWidth: float,
    maxCharWidth: float,
    measuredHeight: float,
    descenderHeight: float,
    smoothing: [@opaque] Revery.Font.Smoothing.t,
    features: [@opaque] list(Revery.Font.Feature.t),
    measurementCache: [@opaque] FontMeasurementCache.t,
  };

let toString = show_font;

let default = Oni_Core.Font.default;

let measure = (~text, v: font) => {
  float_of_int(Zed_utf8.length(text)) *. v.spaceWidth;
};

let getHeight = ({measuredHeight, _}) => measuredHeight;

[@deriving show({with_path: false})]
type msg =
  | FontLoaded(font)
  | FontLoadError(string);

let fontCache = FontResolutionCache.create(~initialSize=1024, 128 * 1024);

let resolveWithFallback = (~italic=false, weight, family) =>
  switch (FontResolutionCache.find((family, weight, italic), fontCache)) {
  | Some(font) => font
  | None =>
    Log.debug("Unable to find font in cache");
    Revery_Font.Family.resolve(~italic, weight, Constants.defaultFontFamily)
    |> Result.get_ok;
  };

let setFont =
    (
      ~requestId,
      ~fontFamily as familyString,
      ~fontSize,
      ~fontLigatures,
      ~fontWeight,
      ~smoothing,
      ~dispatch,
    ) => {
  let dispatch = action => Revery.App.runOnMainThread(() => dispatch(action));

  incr(requestId);
  let req = requestId^;

  Log.infof(m =>
    m(
      "Loading font: %s %s %f %d",
      familyString,
      Revery.Font.Weight.show(fontWeight),
      fontSize,
      req,
    )
  );

  let fontSize = max(fontSize, Constants.minimumFontSize);

  let family =
    if (Constants.isDefaultFont(familyString)) {
      Constants.defaultFontFamily;
    } else if (Rench.Path.isAbsolute(familyString)) {
      Revery_Font.Family.fromFile(familyString);
    } else {
      Revery_Font.Family.system(familyString);
    };

  let features = fontLigatures |> FontLigatures.toHarfbuzzFeatures;

  let res =
    FontLoader.loadAndValidateEditorFont(
      ~requestId=req,
      ~smoothing,
      ~family,
      ~weight=fontWeight,
      ~fontCache,
      fontSize,
    );

  switch (res) {
  | Error(msg) =>
    Log.errorf(m => m("Error loading font: %s %s", familyString, msg));
    dispatch(
      FontLoadError(
        Printf.sprintf("Unable to load font: %s: %s", familyString, msg),
      ),
    );
  | Ok((
      reqId,
      {
        fontFamily,
        fontWeight,
        fontSize,
        spaceWidth,
        underscoreWidth,
        avgCharWidth,
        maxCharWidth,
        measuredHeight,
        descenderHeight,
        smoothing,
        _,
      },
    )) =>
    if (reqId == requestId^) {
      dispatch(
        FontLoaded({
          fontFamily,
          fontWeight,
          fontSize,
          spaceWidth,
          underscoreWidth,
          avgCharWidth,
          maxCharWidth,
          measuredHeight,
          descenderHeight,
          smoothing,
          features,
          measurementCache:
            FontMeasurementCache.create(
              ~fontFamily,
              ~fontWeight,
              ~fontSize,
              ~features,
              ~smoothing,
            ),
        }),
      );
    }
  };
};

module Sub = {
  type params = {
    fontFamily: string,
    fontSize: float,
    fontLigatures: FontLigatures.t,
    fontSmoothing: FontSmoothing.t,
    fontWeight: Revery.Font.Weight.t,
    uniqueId: string,
  };

  module FontSubscription =
    Isolinear.Sub.Make({
      type state = {
        fontFamily: string,
        fontSize: float,
        fontLigatures: FontLigatures.t,
        fontSmoothing: FontSmoothing.t,
        fontWeight: Revery.Font.Weight.t,
        requestId: ref(int),
      };
      type nonrec msg = msg;
      type nonrec params = params;

      let name = "Font";

      let id = ({uniqueId, _}) => uniqueId;

      let getReveryFontSmoothing: FontSmoothing.t => Revery.Font.Smoothing.t =
        fun
        | None => Revery.Font.Smoothing.None
        | Antialiased => Revery.Font.Smoothing.Antialiased
        | SubpixelAntialiased => Revery.Font.Smoothing.SubpixelAntialiased
        | Default => Revery.Font.Smoothing.default;

      let init = (~params: params, ~dispatch: msg => unit) => {
        let reveryFontSmoothing =
          getReveryFontSmoothing(params.fontSmoothing);

        let requestId = ref(0);

        setFont(
          ~requestId,
          ~fontFamily=params.fontFamily,
          ~fontSize=params.fontSize,
          ~fontLigatures=params.fontLigatures,
          ~fontWeight=params.fontWeight,
          ~smoothing=reveryFontSmoothing,
          ~dispatch,
        );

        {
          fontFamily: params.fontFamily,
          fontSize: params.fontSize,
          fontSmoothing: params.fontSmoothing,
          fontLigatures: params.fontLigatures,
          fontWeight: params.fontWeight,
          requestId,
        };
      };

      let update = (~params: params, ~state: state, ~dispatch: msg => unit) =>
        if (params.fontFamily != state.fontFamily
            || !Float.equal(params.fontSize, state.fontSize)
            || params.fontSmoothing != state.fontSmoothing
            || params.fontLigatures != state.fontLigatures
            || params.fontWeight != state.fontWeight) {
          let reveryFontSmoothing =
            getReveryFontSmoothing(params.fontSmoothing);
          setFont(
            ~requestId=state.requestId,
            ~fontFamily=params.fontFamily,
            ~fontSize=params.fontSize,
            ~smoothing=reveryFontSmoothing,
            ~fontLigatures=params.fontLigatures,
            ~fontWeight=params.fontWeight,
            ~dispatch,
          );
          {
            ...state,
            fontFamily: params.fontFamily,
            fontSize: params.fontSize,
            fontWeight: params.fontWeight,
            fontSmoothing: params.fontSmoothing,
            fontLigatures: params.fontLigatures,
          };
        } else {
          state;
        };

      let dispose = (~params as _, ~state) => {
        // Cancel any pending font requests
        state.requestId := (-1);
      };
    });

  let font =
      (
        ~uniqueId,
        ~fontFamily,
        ~fontSize,
        ~fontWeight,
        ~fontLigatures,
        ~fontSmoothing,
      ) => {
    FontSubscription.create({
      uniqueId,
      fontFamily,
      fontSize,
      fontWeight,
      fontSmoothing,
      fontLigatures,
    });
  };
};
