open Oni_Core;

module Zed_utf8 = ZedBundled;
module Log = (val Log.withNamespace("Oni2.Service.Font"));

[@deriving show({with_path: false})]
type font = {
  fontFamily: [@opaque] Revery.Font.Family.t,
  fontSize: float,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: [@opaque] Revery.Font.Smoothing.t,
};

let toString = show_font;

let default = {
  fontFamily: Revery.Font.Family.fromFile(Constants.defaultFontFile),
  fontSize: Constants.defaultFontSize,
  measuredWidth: 1.,
  measuredHeight: 1.,
  descenderHeight: 0.,
  smoothing: Revery.Font.Smoothing.default,
};

let measure = (~text, v: font) => {
  float_of_int(Zed_utf8.length(text)) *. v.measuredWidth;
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
      ~smoothing,
      ~dispatch,
    ) => {
  let dispatch = action => Revery.App.runOnMainThread(() => dispatch(action));

  incr(requestId);
  let req = requestId^;

  Log.infof(m => m("Loading font: %s %f %d", familyString, fontSize, req));

  // We load the font asynchronously
  ThreadHelper.create(
    ~name="FontStore.loadThread",
    () => {
      let fontSize = max(fontSize, Constants.minimumFontSize);

      let family =
        if (familyString == Constants.defaultFontFile) {
          Constants.defaultFontFamily;
        } else if (Rench.Path.isAbsolute(familyString)) {
          Revery_Font.Family.fromFile(familyString);
        } else {
          Revery_Font.Family.system(familyString);
        };

      let res =
        FontLoader.loadAndValidateEditorFont(
          ~requestId=req,
          ~smoothing,
          ~family,
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
            fontSize,
            measuredWidth,
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
              fontSize,
              measuredWidth,
              measuredHeight,
              descenderHeight,
              smoothing,
            }),
          );
        }
      };
    },
    (),
  )
  |> ignore;
};

module Sub = {
  type params = {
    fontFamily: string,
    fontSize: float,
    fontSmoothing: ConfigurationValues.fontSmoothing,
    uniqueId: string,
  };

  module FontSubscription =
    Isolinear.Sub.Make({
      type state = {
        fontFamily: string,
        fontSize: float,
        fontSmoothing: ConfigurationValues.fontSmoothing,
        requestId: ref(int),
      };
      type nonrec msg = msg;
      type nonrec params = params;

      let name = "Font";

      let id = ({uniqueId, _}) => uniqueId;

      let getReveryFontSmoothing:
        ConfigurationValues.fontSmoothing => Revery.Font.Smoothing.t =
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
          ~smoothing=reveryFontSmoothing,
          ~dispatch,
        );

        {
          fontFamily: params.fontFamily,
          fontSize: params.fontSize,
          fontSmoothing: params.fontSmoothing,
          requestId,
        };
      };

      let update = (~params: params, ~state: state, ~dispatch: msg => unit) =>
        if (params.fontFamily != state.fontFamily
            || !Float.equal(params.fontSize, state.fontSize)
            || params.fontSmoothing != state.fontSmoothing) {
          let reveryFontSmoothing =
            getReveryFontSmoothing(params.fontSmoothing);
          setFont(
            ~requestId=state.requestId,
            ~fontFamily=params.fontFamily,
            ~fontSize=params.fontSize,
            ~smoothing=reveryFontSmoothing,
            ~dispatch,
          );
          {
            ...state,
            fontFamily: params.fontFamily,
            fontSize: params.fontSize,
            fontSmoothing: params.fontSmoothing,
          };
        } else {
          state;
        };

      let dispose = (~params as _, ~state) => {
        // Cancel any pending font requests
        state.requestId := (-1);
      };
    });

  let font = (~uniqueId, ~fontFamily, ~fontSize, ~fontSmoothing) => {
    FontSubscription.create({uniqueId, fontFamily, fontSize, fontSmoothing});
  };
};
