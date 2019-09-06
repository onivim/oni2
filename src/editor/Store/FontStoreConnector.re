/*
 * FontStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing fonts
 */

open Oni_Core;
open Oni_Model;

let minFontSize = 6;
let defaultFontFamily = "FiraCode-Regular.ttf";
let defaultFontSize = 14;

let loadAndValidateEditorFont =
    (~onSuccess, ~onError, scaleFactor, fullPath, fontSize) => {
  Log.info(
    "loadAndValidateEditorFont filePath: " ++ fullPath ++ " | size: " ++ string_of_int(fontSize),
  );

  let adjSize = int_of_float(float_of_int(fontSize) *. scaleFactor +. 0.5);

  Fontkit.fk_new_face(
    fullPath,
    adjSize,
    font => {
      open Types;

      /* Measure text */
      let shapedText = Fontkit.fk_shape(font, "Hi");
      let firstShape = shapedText[0];
      let secondShape = shapedText[1];

      let glyph = Fontkit.renderGlyph(font, firstShape.glyphId);
      let secondGlyph = Fontkit.renderGlyph(font, secondShape.glyphId);

      if (glyph.advance != secondGlyph.advance) {
        onError("Not a monospace font.");
      } else {
        let metrics = Fontkit.fk_get_metrics(font);
        let actualHeight =
          float_of_int(fontSize)
          *. float_of_int(metrics.height)
          /. float_of_int(metrics.unitsPerEm);

        let measuredWidth =
          float_of_int(glyph.advance) /. (64. *. scaleFactor);
        let measuredHeight = floor(actualHeight +. 0.5);

        Log.info(
          "Font loaded! Measured width: "
          ++ string_of_float(measuredWidth)
          ++ " Measured height: "
          ++ string_of_float(measuredHeight),
        );
        /* Set editor text based on measurements */
        onSuccess(
          EditorFont.create(
            ~fontFile=fullPath,
            ~fontSize,
            ~measuredWidth,
            ~measuredHeight,
            (),
          ),
        );
      };
    },
    _ => onError("Unable to load font."),
  );
};

let start = (~getScaleFactor, ()) => {
  let setFont = (dispatch1, fontFamily, fontSize) => {
    let dispatch = action =>
      Revery.App.runOnMainThread(() => dispatch1(action));

    let scaleFactor = getScaleFactor();
  

    // We load the font asynchronously
    let _ =
      Thread.create(
        () => {
          let fontSize = max(fontSize, minFontSize);

          let (name, fullPath) =
            switch (fontFamily) {
            | None => (
                defaultFontFamily,
                Utility.executingDirectory ++ defaultFontFamily,
              )
            | Some(v) =>
              Log.info("FontStoreConnector::setFont - discovering font: " ++ v);
              let descriptor =
                Revery.Font.find(
                  ~mono=true,
                  ~weight=Revery.Font.Weight.Normal,
                  v,
                );
              Log.info("FontStoreConnector::setFont - discovering font at path: " ++ descriptor.path);
              (v, descriptor.path);
            };

          let onSuccess = editorFont => {
            dispatch(Actions.SetEditorFont(editorFont));
          };

          let onError = errorMsg => {
            Log.error("setFont: Failed to load font " ++ fullPath);

            dispatch(
              ShowNotification(
                Notification.create(
                  ~notificationType=Actions.Error,
                  ~title="Unable to load font",
                  ~message=name ++ ": " ++ errorMsg,
                  (),
                ),
              ),
            );
          };

          loadAndValidateEditorFont(
            ~onSuccess,
            ~onError,
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

      Log.info("FontStoreConnector::synchronizeConfiguration");

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
