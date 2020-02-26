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

let requestId = ref(0);

let setFont = (dispatch1, fontFamily, fontSize, smoothing) => {
  let dispatch = action =>
    Revery.App.runOnMainThread(() => dispatch1(action));

  incr(requestId);
  let req = requestId^;

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
  };

  module FontSubscription =
    Isolinear.Sub.Make({
      type state = unit;
      type nonrec msg = msg;
      type nonrec params = params;

      let subscriptionName = "Font";

      let getUniqueId = ({fontFamily, fontSize, fontSmoothing}) => {
        Printf.sprintf(
          "%s-%s-%s",
          fontFamily,
          string_of_float(fontSize),
          ConfigurationValues.show_fontSmoothing(fontSmoothing),
        );
      };

      let init = (~params, ~dispatch) => {
        let reveryFontSmoothing =
          switch (params.fontSmoothing) {
          | None => Revery.Font.Smoothing.None
          | Antialiased => Revery.Font.Smoothing.Antialiased
          | SubpixelAntialiased => Revery.Font.Smoothing.SubpixelAntialiased
          | Default => Revery.Font.Smoothing.default
          };

        setFont(
          dispatch,
          params.fontFamily,
          params.fontSize,
          reveryFontSmoothing,
        );
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;
      let dispose = (~params as _, ~state as _) => ();
    });

  let font = (~fontFamily, ~fontSize, ~fontSmoothing) => {
    FontSubscription.create({fontFamily, fontSize, fontSmoothing});
  };
};
