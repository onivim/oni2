open Oni_Core;
open Oni_Model;
open BenchFramework;
open Feature_Editor;

open Helpers;

let setup = () => {
  let hwnd =
    Revery.Utility.HeadlessWindow.create(
      Revery.WindowCreateOptions.create(~width=3440, ~height=1440, ()),
    );
  hwnd;
};

let configResolver =
    (~fileType as _, settings, _vimModel, ~vimSetting as _, key) =>
  Config.Settings.get(key, settings)
  |> Option.map(configJson => Config.Json(configJson))
  |> Option.value(~default=Config.NotSet);

let editor = (editor, buffer, state: State.t) => {
  <EditorSurface
    isActiveSplit=true
    languageConfiguration=LanguageConfiguration.default
    dispatch={_ => ()}
    editor
    buffer
    onEditorSizeChanged={(_, _, _) => ()}
    bufferHighlights={state.bufferHighlights}
    bufferSyntaxHighlights={state.syntaxHighlights}
    diagnostics={state.diagnostics}
    tokenTheme={state.colorTheme |> Feature_Theme.tokenColors}
    languageSupport={state.languageSupport}
    theme={Feature_Theme.colors(state.colorTheme)}
    windowIsFocused=true
    buffers=Feature_Buffers.empty
    perFileTypeConfig={configResolver(
      Config.Settings.empty,
      Feature_Vim.initial,
    )}
    languageInfo=Exthost.LanguageInfo.initial
    grammarRepository=Oni_Syntax.GrammarRepository.empty
    uiFont=Oni_Core.UiFont.default
    snippets=Feature_Snippets.initial
    renderOverlays={(~gutterWidth as _) => <Revery.UI.View />}
  />;
};

let editorSurfaceMinimalState = hwnd => {
  Revery.Utility.HeadlessWindow.render(
    hwnd,
    editor(simpleEditor, thousandLineBuffer, thousandLineState),
  );
};

let editorSurfaceThousandLineState = hwnd => {
  Revery.Utility.HeadlessWindow.render(
    hwnd,
    editor(simpleEditor, thousandLineBuffer, thousandLineState),
  );
};

let editorSurfaceThousandLineStateWithIndents = hwnd => {
  Revery.Utility.HeadlessWindow.render(
    hwnd,
    editor(simpleEditor, thousandLineBuffer, thousandLineState),
  );
  ();
};
let editorSurfaceHundredThousandLineStateNoMinimap = hwnd => {
  Revery.Utility.HeadlessWindow.render(
    hwnd,
    editor(simpleEditor, thousandLineBuffer, thousandLineState),
  );
};

let editorSurfaceHundredThousandLineState = hwnd => {
  Revery.Utility.HeadlessWindow.render(
    hwnd,
    editor(simpleEditor, thousandLineBuffer, thousandLineState),
  );
};

let runUIBench = (~name, ~screenshotFile, f) => {
  let setupWithScreenshot = () => {
    let hwnd = setup();

    switch (Sys.getenv_opt("ONI2_BENCH_SCREENSHOT")) {
    | None => ()
    | Some(_) =>
      print_endline(
        "Outputting screenshot for: " ++ name ++ " as " ++ screenshotFile,
      );
      f(hwnd);
      Revery.Utility.HeadlessWindow.takeScreenshot(hwnd, screenshotFile);
    };

    hwnd;
  };

  let options = Reperf.Options.create(~iterations=10, ());

  bench(~name, ~options, ~setup=setupWithScreenshot, ~f, ());
};

runUIBench(
  ~name="EditorSurface - Rendering: Minimal state",
  ~screenshotFile="minimal.png",
  editorSurfaceMinimalState,
);

runUIBench(
  ~name="EditorSurface - Rendering: 1000 Lines state",
  ~screenshotFile="thousand.png",
  editorSurfaceThousandLineState,
);

runUIBench(
  ~name="EditorSurface - Rendering: 1000 Lines state - With indent guides",
  ~screenshotFile="thousand_plus_indents.png",
  editorSurfaceThousandLineStateWithIndents,
);

runUIBench(
  ~name="EditorSurface - Rendering: 100000 Lines state",
  ~screenshotFile="hundred_thousand.png",
  editorSurfaceHundredThousandLineState,
);

runUIBench(
  ~name="EditorSurface - Rendering: 100000 Lines state - No minimap",
  ~screenshotFile="hundred_thousand_no_minimap.png",
  editorSurfaceHundredThousandLineStateNoMinimap,
);
