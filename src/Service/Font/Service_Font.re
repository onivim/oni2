open Oni_Core;

module Zed_utf8 = ZedBundled;
module Log = (val Log.withNamespace("Oni2.Service.Font"));

[@deriving show({with_path: false})]
type t = {
  fontFile: string,
  fontSize: float,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: [@opaque] Revery.Font.Smoothing.t,
};

let toString = show;

let default = {
  fontFile: Constants.defaultFontFamily,
  fontSize: Constants.defaultFontSize,
  measuredWidth: 1.,
  measuredHeight: 1.,
  descenderHeight: 0.,
  smoothing: Revery.Font.Smoothing.default,
};

let measure = (~text, v: t) => {
  float_of_int(Zed_utf8.length(text)) *. v.measuredWidth;
};

let getHeight = ({measuredHeight, _}) => measuredHeight;

[@deriving show({with_path: false})]
type msg =
  | FontLoaded(t)
  | FontLoadError(string);

let setFont = (requestId, dispatch1, fontFamily, fontSize, smoothing) => {
  let dispatch = action =>
    Revery.App.runOnMainThread(() => dispatch1(action));

  incr(requestId);
  let req = requestId^;

  Log.infof(m => m("Loading font: %s %f %d", fontFamily, fontSize, req));

  // We load the font asynchronously
  ThreadHelper.create(
    ~name="FontStore.loadThread",
    () => {
      let fontSize = max(fontSize, Constants.minimumFontSize);

      let (name, fullPath) =
        if (fontFamily == Constants.defaultFontFamily) {
          (
            Constants.defaultFontFamily,
            Revery.Environment.executingDirectory
            ++ Constants.defaultFontFamily,
          );
        } else {
          Log.debug("Discovering font: " ++ fontFamily);

          if (Rench.Path.isAbsolute(fontFamily)) {
            (fontFamily, fontFamily);
          } else {
            let descriptor =
              Revery.Font.Discovery.find(
                ~mono=true,
                ~weight=Revery.Font.Weight.Normal,
                fontFamily,
              );

            Log.debug("  at path: " ++ descriptor.path);

            (fontFamily, descriptor.path);
          };
        };

      let res =
        FontLoader.loadAndValidateEditorFont(
          ~requestId=req,
          ~smoothing,
          fullPath,
          fontSize,
        );

      switch (res) {
      | Error(msg) =>
        Log.errorf(m => m("Error loading font: %s %s", name, msg));
        dispatch(
          FontLoadError(
            Printf.sprintf("Unable to load font: %s: %s", name, msg),
          ),
        );
      | Ok((
          reqId,
          {
            fontFile,
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
              fontFile,
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

      let subscriptionName = "Font";

      let getUniqueId = ({uniqueId, _}) => uniqueId;

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
          requestId,
          dispatch,
          params.fontFamily,
          params.fontSize,
          reveryFontSmoothing,
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
            state.requestId,
            dispatch,
            params.fontFamily,
            params.fontSize,
            reveryFontSmoothing,
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
