open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Service.Font"));

[@deriving show({with_path: false})]
type t = {
  fontFamily: string,
  fontSize: float,
  font: [@opaque] Revery.Font.t,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: [@opaque] Revery.Font.Smoothing.t,
};

[@deriving show({with_path: false})]
type msg =
  | FontLoaded(t)
  | FontLoadError(string);

let requestId = ref(0);

let setFont = (dispatch1, fontFamily, fontSize: float) => {
  let dispatch = action =>
    Revery.App.runOnMainThread(() => dispatch1(action));

  incr(requestId);
  let req = requestId^;

  // We load the font asynchronously
  let _: Thread.t =
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
            fullPath,
            fontSize,
          );

        switch (res) {
        | Error(msg) =>
          dispatch(
            FontLoadError(
              Printf.sprintf("Unable to load font: %s: %s", name, msg),
            ),
          )
        | Ok((reqId, editorFont)) =>
          if (reqId == requestId^) {
            ()// TODO:
              ;
              /*dispatch(FontLoaded(
                  editorFont
                );*/
          }
        };
      },
      (),
    );
  ();
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
        print_endline("FontSubscription: INIT: " ++ getUniqueId(params));
        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as _) => {
        print_endline("FontSubscription: DISPOSE");
        ();
      };
    });

  let font = (~fontFamily, ~fontSize, ~fontSmoothing) => {
    FontSubscription.create({fontFamily, fontSize, fontSmoothing});
  };
};
