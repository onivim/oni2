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

let editorSurfaceMinimalState = hwnd => {
  Revery.Utility.HeadlessWindow.render(
    hwnd,
    <EditorSurface
      isActiveSplit=true
      editor=simpleEditor
      buffer=thousandLineBuffer
      metrics
      onScroll={_ => ()}
      onDimensionsChanged={_ => ()}
      onCursorChange={_ => ()}
      bufferHighlights={thousandLineState.bufferHighlights}
      bufferSyntaxHighlights={thousandLineState.syntaxHighlights}
      diagnostics={thousandLineState.diagnostics}
      completions={thousandLineState.completions}
      tokenTheme={thousandLineState.tokenTheme}
      definition={thousandLineState.definition}
      mode={thousandLineState.mode}
      theme={thousandLineState.theme}
      editorFont={thousandLineState.editorFont}
      windowIsFocused=true
    />,
  );
};

let editorSurfaceThousandLineState = hwnd => {
  Revery.Utility.HeadlessWindow.render(
    hwnd,
    <EditorSurface
      isActiveSplit=true
      editor=simpleEditor
      buffer=thousandLineBuffer
      metrics
      onScroll={_ => ()}
      onDimensionsChanged={_ => ()}
      onCursorChange={_ => ()}
      bufferHighlights={thousandLineState.bufferHighlights}
      bufferSyntaxHighlights={thousandLineState.syntaxHighlights}
      diagnostics={thousandLineState.diagnostics}
      completions={thousandLineState.completions}
      tokenTheme={thousandLineState.tokenTheme}
      definition={thousandLineState.definition}
      mode={thousandLineState.mode}
      theme={thousandLineState.theme}
      editorFont={thousandLineState.editorFont}
      windowIsFocused=true
    />,
  );
};

let editorSurfaceThousandLineStateWithIndents = hwnd => {
  Revery.Utility.HeadlessWindow.render(
    hwnd,
    <EditorSurface
      isActiveSplit=true
      editor=simpleEditor
      buffer=thousandLineBuffer
      metrics
      onScroll={_ => ()}
      onDimensionsChanged={_ => ()}
      onCursorChange={_ => ()}
      bufferHighlights={thousandLineState.bufferHighlights}
      bufferSyntaxHighlights={thousandLineState.syntaxHighlights}
      diagnostics={thousandLineState.diagnostics}
      completions={thousandLineState.completions}
      tokenTheme={thousandLineState.tokenTheme}
      definition={thousandLineState.definition}
      mode={thousandLineState.mode}
      theme={thousandLineState.theme}
      editorFont={thousandLineState.editorFont}
      shouldRenderIndentGuides=true
      windowIsFocused=true
    />,
  );
  ();
};
let editorSurfaceHundredThousandLineStateNoMinimap = hwnd => {
  Revery.Utility.HeadlessWindow.render(
    hwnd,
    <EditorSurface
      isActiveSplit=true
      editor=simpleEditor
      buffer=thousandLineBuffer
      metrics
      onScroll={_ => ()}
      onDimensionsChanged={_ => ()}
      onCursorChange={_ => ()}
      bufferHighlights={thousandLineState.bufferHighlights}
      bufferSyntaxHighlights={thousandLineState.syntaxHighlights}
      diagnostics={thousandLineState.diagnostics}
      completions={thousandLineState.completions}
      tokenTheme={thousandLineState.tokenTheme}
      definition={thousandLineState.definition}
      mode={thousandLineState.mode}
      theme={thousandLineState.theme}
      editorFont={thousandLineState.editorFont}
      showMinimap=false
      windowIsFocused=true
    />,
  );
};

let editorSurfaceHundredThousandLineState = hwnd => {
  Revery.Utility.HeadlessWindow.render(
    hwnd,
    <EditorSurface
      isActiveSplit=true
      editor=simpleEditor
      buffer=thousandLineBuffer
      metrics
      onScroll={_ => ()}
      onDimensionsChanged={_ => ()}
      onCursorChange={_ => ()}
      bufferHighlights={thousandLineState.bufferHighlights}
      bufferSyntaxHighlights={thousandLineState.syntaxHighlights}
      diagnostics={thousandLineState.diagnostics}
      completions={thousandLineState.completions}
      tokenTheme={thousandLineState.tokenTheme}
      definition={thousandLineState.definition}
      mode={thousandLineState.mode}
      theme={thousandLineState.theme}
      editorFont={thousandLineState.editorFont}
      windowIsFocused=true
    />,
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
