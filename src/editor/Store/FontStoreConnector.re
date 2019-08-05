/*
 * FontStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing fonts
 */

open Oni_Core;
open Oni_Model;

let start = () => {
  let setFont = (dispatch, fontFamily, fontSize) => {
    let scaleFactor =
      Window.getDevicePixelRatio(w)
      *. float_of_int(Window.getScaleFactor(w));

    let adjSize = int_of_float(float_of_int(fontSize) *. scaleFactor +. 0.5);

    let fontFile = Core.Utility.executingDirectory ++ fontFamily;

    Log.info("Loading font: " ++ fontFile);

    Fontkit.fk_new_face(
      fontFile,
      adjSize,
      font => {
        Log.info("Font loaded!");
        open Oni_Model.Actions;
        open Oni_Core.Types;

        /* Measure text */
        let shapedText = Fontkit.fk_shape(font, "H");
        let firstShape = shapedText[0];
        let glyph = Fontkit.renderGlyph(font, firstShape.glyphId);

        let metrics = Fontkit.fk_get_metrics(font);
        let actualHeight =
          float_of_int(fontSize)
          *. float_of_int(metrics.height)
          /. float_of_int(metrics.unitsPerEm);

        /* Set editor text based on measurements */
        dispatch(
          SetEditorFont(
            EditorFont.create(
              ~fontFile=fontFamily,
              ~fontSize,
              ~measuredWidth=
                float_of_int(glyph.advance) /. (64. *. scaleFactor),
              ~measuredHeight=floor(actualHeight +. 0.5),
              (),
            ),
          ),
        );
      },
      _ => Log.error("setFont: Failed to load font " ++ fontFamily),
    );
  };
  
  let synchronizeConfiguration = (configuration: Core.Configuration.t) =>
    Isolinear.Effect.create(~name="windows.syncConfig", () => {
    
    let editorFontFamily = "FiraCode-Regular.ttf";

    let editorFontSize = Core.Configuration.getValue(
      c => c.editorFontSize,
      configuration,
    );

    setFont(dispatch, editorFontFamily, editorFontSize),
  });
  
  let loadFontEffect = (fontFamily, fontSize) =>
    Isolinear.Effect.createWithDispatch(
    ~name="font.loadEditorFont", dispatch => {
      print_endline ("Trying to load font: " ++ fontFamily);
    }
  );
  
  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.ConfigurationSet(c) => synchronizeConfiguration(c)
    | Actions.LoadEditorFont(fontFamily, fontSize) => (state, loadEditorFontEffect(fontFamily, fontSize))
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
