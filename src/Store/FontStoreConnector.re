/*
 * FontStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing fonts
 */

open Oni_Core;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Store.Font"));

let minFontSize = 6;
let defaultFontFamily = "FiraCode-Regular.ttf";
let defaultFontSize = 14;

let requestId = ref(0);

let loadAndValidateEditorFont =
    (~onSuccess, ~onError, ~requestId: int, scaleFactor, fullPath, fontSize) => {
  Log.debugf(m =>
    m("loadAndValidateEditorFont path: %s | size: %i", fullPath, fontSize)
  );

  let adjSize = int_of_float(float_of_int(fontSize) *. scaleFactor +. 0.5);

  Fontkit.fk_new_face(
    fullPath,
    adjSize,
    font => {
      /* Measure text */
      let shapedText = Fontkit.fk_shape(font, "Hi");
      let firstShape = shapedText[0];
      let secondShape = shapedText[1];

      Log.debugf(m =>
        m("glyph1: %i glyph2: %i", firstShape.glyphId, secondShape.glyphId)
      );

      let glyph = Fontkit.renderGlyph(font, firstShape.glyphId);
      Log.debug("Got glyph for firstShape");
      let secondGlyph = Fontkit.renderGlyph(font, secondShape.glyphId);
      Log.debug("Got glyph for secondShape");

      if (glyph.advance != secondGlyph.advance) {
        onError("Not a monospace font.");
      } else if (firstShape.glyphId == secondShape.glyphId) {
        onError("Unable to load glyphs.");
      } else {
        let metrics = Fontkit.fk_get_metrics(font);
        let actualHeight =
          float_of_int(fontSize)
          *. float_of_int(metrics.height)
          /. float_of_int(metrics.unitsPerEm);

        let measuredWidth =
          float_of_int(glyph.advance) /. (64. *. scaleFactor);
        let measuredHeight = floor(actualHeight +. 0.5);

        Log.debugf(m => m("Measured width: %f ", measuredWidth));
        Log.debugf(m => m("Measured height: %f ", measuredHeight));

        /* Set editor text based on measurements */
        onSuccess((
          requestId,
          EditorFont.create(
            ~fontFile=fullPath,
            ~fontSize,
            ~measuredWidth,
            ~measuredHeight,
            (),
          ),
        ));
      };
    },
    _ => onError("Unable to load font."),
  );
};

let start = (~getScaleFactor, ()) => {
  let setFont = (dispatch1, maybeFontFamily, fontSize) => {
    let dispatch = action =>
      Revery.App.runOnMainThread(() => dispatch1(action));

    let scaleFactor = getScaleFactor();

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
                Utility.executingDirectory ++ defaultFontFamily,
              )

            | Some(fontFamily) when fontFamily == "FiraCode-Regular.ttf" => (
                defaultFontFamily,
                Utility.executingDirectory ++ defaultFontFamily,
              )

            | Some(fontFamily) =>
              Log.debug("Discovering font: " ++ fontFamily);

              if (Rench.Path.isAbsolute(fontFamily)) {
                (fontFamily, fontFamily);
              } else {
                let descriptor =
                  Revery.Font.find(
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
            scaleFactor,
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
      // TODO
      let editorFontFamily =
        Configuration.getValue(c => c.editorFontFamily, configuration);

      let editorFontSize =
        Configuration.getValue(c => c.editorFontSize, configuration);

      Log.debug("synchronizeConfiguration");

      setFont(dispatch, editorFontFamily, editorFontSize);
    });

  let loadEditorFontEffect = (fontFamily, fontSize) =>
    Isolinear.Effect.createWithDispatch(~name="font.loadEditorFont", dispatch =>
      setFont(dispatch, fontFamily, fontSize)
    );

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Init => (state, loadEditorFontEffect(None, defaultFontSize))
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
