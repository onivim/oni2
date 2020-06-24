open Oni_Core;
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

let configResolver = (settings, key) => Config.Settings.get(key, settings);

let editor = (editor, buffer, state: State.t) => {
    <EditorSurface
      isActiveSplit=true
      languageConfiguration=LanguageConfiguration.default
      dispatch={_ => ()}
      editor
      buffer
      onEditorSizeChanged={(_, _, _) => ()}
      onCursorChange={_ => ()}
      bufferHighlights={state.bufferHighlights}
      bufferSyntaxHighlights={state.syntaxHighlights}
      diagnostics={state.diagnostics}
      completions={state.completions}
      tokenTheme={state.tokenTheme}
      definition={state.definition}
      mode={Feature_Vim.mode(state.vim)}
      theme={Feature_Theme.colors(state.colorTheme)}
      windowIsFocused=true
      config={configResolver(Config.Settings.empty)}
    />
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
