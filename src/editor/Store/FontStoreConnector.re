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

let start = (~getScaleFactor, ()) => {
  let setFont = (dispatch, fontFamily, fontSize) => {
    let fontSize = max(fontSize, minFontSize);
    let scaleFactor = getScaleFactor();
    let adjSize = int_of_float(float_of_int(fontSize) *. scaleFactor +. 0.5);

    let fontFile = Utility.executingDirectory ++ fontFamily;

    Log.info(
      "Loading font: " ++ fontFile ++ " | size: " ++ string_of_int(fontSize),
    );

    Fontkit.fk_new_face(
      fontFile,
      adjSize,
      font => {
        open Oni_Model.Actions;
        open Types;

        /* Measure text */
        let shapedText = Fontkit.fk_shape(font, "H");
        let firstShape = shapedText[0];
        let glyph = Fontkit.renderGlyph(font, firstShape.glyphId);

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
        dispatch(
          SetEditorFont(
            EditorFont.create(
              ~fontFile=fontFamily,
              ~fontSize,
              ~measuredWidth,
              ~measuredHeight,
              (),
            ),
          ),
        );
      },
      _ => Log.error("setFont: Failed to load font " ++ fontFamily),
    );
  };

  let synchronizeConfiguration = (configuration: Configuration.t) =>
    Isolinear.Effect.createWithDispatch(~name="windows.syncConfig", dispatch => {
      // TODO
      /* let editorFontFamily =
         Configuration.getValue(c => c.editorFontFamily, configuration); */

      let editorFontSize =
        Configuration.getValue(c => c.editorFontSize, configuration);

      setFont(dispatch, defaultFontFamily, editorFontSize);
    });

  let loadEditorFontEffect = (fontFamily, fontSize) =>
    Isolinear.Effect.createWithDispatch(~name="font.loadEditorFont", dispatch =>
      setFont(dispatch, fontFamily, fontSize)
    );

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Init => (
        state,
        loadEditorFontEffect(defaultFontFamily, defaultFontSize),
      )
    | Actions.ConfigurationSet(c) => (state, synchronizeConfiguration(c))
    | Actions.LoadEditorFont(fontFamily, fontSize) => (
        state,
        loadEditorFontEffect(fontFamily, fontSize),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
