/*
 * FontStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing fonts
 */

open Oni_Core;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Store.Font"));

let minFontSize = 6.;
let defaultFontFamily = "FiraCode-Regular.ttf";
let defaultFontSize = 14.;

let requestId = ref(0);

let loadAndValidateEditorFont =
    (~onSuccess, ~onError, ~requestId: int, fullPath, fontSize: float) => {
  Log.tracef(m =>
    m("loadAndValidateEditorFont path: %s | size: %f", fullPath, fontSize)
  );

  let fontResult = Revery.Font.FontCache.load(fullPath);

  switch (fontResult) {
  | Error(msg) => onError(msg)
  | Ok(font) =>
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
      onError("Not a monospace font");
    } else {
      let measuredWidth = character1.width;
      let {lineHeight, descent, _}: Revery.Font.FontMetrics.t =
        Revery.Font.getMetrics(font, fontSize);

      Log.debugf(m => m("Measured width: %f ", measuredWidth));
      Log.debugf(m => m("Line height: %f ", lineHeight));

      onSuccess((
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
  };
};

let start = () => {
  let setFont = (dispatch1, maybeFontFamily, fontSize: float) => {
    let dispatch = action =>
      Revery.App.runOnMainThread(() => dispatch1(action));

    incr(requestId);
    let req = requestId^;

    // We load the font asynchronously
    let _ =
      ThreadHelper.create(
        ~name="FontStore.loadThread",
        () => {
          let fontSize = max(fontSize, minFontSize);

          let (name, fullPath) =
            switch (maybeFontFamily) {
            | None => (
                defaultFontFamily,
                Revery.Environment.executingDirectory ++ defaultFontFamily,
              )

            | Some(fontFamily) when fontFamily == "FiraCode-Regular.ttf" => (
                defaultFontFamily,
                Revery.Environment.executingDirectory ++ defaultFontFamily,
              )

            | Some(fontFamily) =>
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

          let onSuccess = ((reqId, editorFont)) =>
            if (reqId == requestId^) {
              dispatch(Actions.SetEditorFont(editorFont));
            };

          let onError = errorMsg => {
            Log.error("Failed to load font " ++ fullPath);

            dispatch(
              ShowNotification(
                Notification.create(
                  ~kind=Error,
                  Printf.sprintf(
                    "Unable to load font: %s: %s",
                    name,
                    errorMsg,
                  ),
                ),
              ),
            );
          };

          loadAndValidateEditorFont(
            ~onSuccess,
            ~onError,
            ~requestId=req,
            fullPath,
            fontSize,
          );
        },
        (),
      );
    ();
  };

  let synchronizeConfiguration = (configuration: Configuration.t) =>
    Isolinear.Effect.createWithDispatch(~name="windows.syncConfig", dispatch => {
      Log.trace("synchronizeConfiguration");

      // TODO
      let editorFontFamily =
        Configuration.getValue(c => c.editorFontFamily, configuration);

      let editorFontSize =
        Configuration.getValue(c => c.editorFontSize, configuration);

      setFont(dispatch, editorFontFamily, editorFontSize);
    });

  let loadEditorFontEffect = (fontFamily, fontSize) =>
    Isolinear.Effect.createWithDispatch(~name="font.loadEditorFont", dispatch =>
      setFont(dispatch, fontFamily, fontSize)
    );

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Init(_) => (state, loadEditorFontEffect(None, defaultFontSize))
    | Actions.ConfigurationSet(c) => (state, synchronizeConfiguration(c))
    | Actions.LoadEditorFont(fontFamily, fontSize) => (
        state,
        loadEditorFontEffect(Some(fontFamily), fontSize),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
