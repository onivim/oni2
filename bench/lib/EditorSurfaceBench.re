open Oni_Model;
open BenchFramework;
open Feature_Editor;

open Helpers;

open Revery.UI;

let rootNode =
  React.RenderedElement.{
    node: (new node)(),
    insertNode: React.insertNode,
    deleteNode: React.deleteNode,
    moveNode: React.moveNode,
  };

let setup = () => ();

let editorSurfaceMinimalState = () => {
  let _ =
    React.RenderedElement.render(
      rootNode,
      <EditorSurface
        isActiveSplit=true
        editor=simpleEditor
        activeBuffer={Selectors.getBufferForEditor(
          thousandLineState,
          simpleEditor,
        )}
        metrics
        onScroll={_ => ()}
        onDimensionsChanged={_ => ()}
        onCursorChange={_ => ()}
        bufferHighlights={thousandLineState.bufferHighlights}
        bufferSyntaxHighlights={thousandLineState.bufferSyntaxHighlights}
        diagnostics={thousandLineState.diagnostics}
        completions={thousandLineState.completions}
        tokenTheme={thousandLineState.tokenTheme}
        definition={thousandLineState.definition}
        mode={thousandLineState.mode}
        theme={thousandLineState.theme}
        editorFont={thousandLineState.editorFont}
      />,
    );
  ();
};

let editorSurfaceThousandLineState = () => {
  let _ =
    React.RenderedElement.render(
      rootNode,
      <EditorSurface
        isActiveSplit=true
        editor=simpleEditor
        activeBuffer={Selectors.getBufferForEditor(
          thousandLineState,
          simpleEditor,
        )}
        metrics
        onScroll={_ => ()}
        onDimensionsChanged={_ => ()}
        onCursorChange={_ => ()}
        bufferHighlights={thousandLineState.bufferHighlights}
        bufferSyntaxHighlights={thousandLineState.bufferSyntaxHighlights}
        diagnostics={thousandLineState.diagnostics}
        completions={thousandLineState.completions}
        tokenTheme={thousandLineState.tokenTheme}
        definition={thousandLineState.definition}
        mode={thousandLineState.mode}
        theme={thousandLineState.theme}
        editorFont={thousandLineState.editorFont}
      />,
    );
  ();
};

let editorSurfaceThousandLineStateWithIndents = () => {
  let _ =
    React.RenderedElement.render(
      rootNode,
      <EditorSurface
        isActiveSplit=true
        editor=simpleEditor
        activeBuffer={Selectors.getBufferForEditor(
          thousandLineState,
          simpleEditor,
        )}
        metrics
        onScroll={_ => ()}
        onDimensionsChanged={_ => ()}
        onCursorChange={_ => ()}
        bufferHighlights={thousandLineState.bufferHighlights}
        bufferSyntaxHighlights={thousandLineState.bufferSyntaxHighlights}
        diagnostics={thousandLineState.diagnostics}
        completions={thousandLineState.completions}
        tokenTheme={thousandLineState.tokenTheme}
        definition={thousandLineState.definition}
        mode={thousandLineState.mode}
        theme={thousandLineState.theme}
        editorFont={thousandLineState.editorFont}
      />,
    );
  ();
};

let editorSurfaceHundredThousandLineState = () => {
  let _ =
    React.RenderedElement.render(
      rootNode,
      <EditorSurface
        isActiveSplit=true
        editor=simpleEditor
        activeBuffer={Selectors.getBufferForEditor(
          thousandLineState,
          simpleEditor,
        )}
        metrics
        onScroll={_ => ()}
        onDimensionsChanged={_ => ()}
        onCursorChange={_ => ()}
        bufferHighlights={thousandLineState.bufferHighlights}
        bufferSyntaxHighlights={thousandLineState.bufferSyntaxHighlights}
        diagnostics={thousandLineState.diagnostics}
        completions={thousandLineState.completions}
        tokenTheme={thousandLineState.tokenTheme}
        definition={thousandLineState.definition}
        mode={thousandLineState.mode}
        theme={thousandLineState.theme}
        editorFont={thousandLineState.editorFont}
      />,
    );
  ();
};

let setupSurfaceThousandLineLayout = () => {
  let rootNode = (new viewNode)();

  rootNode#setStyle(
    Style.make(~position=LayoutTypes.Relative, ~width=1600, ~height=1200, ()),
  );

  let container = Container.create(rootNode);
  Container.update(
    container,
    <EditorSurface
      isActiveSplit=true
      editor=simpleEditor
      activeBuffer={Selectors.getBufferForEditor(
        thousandLineState,
        simpleEditor,
      )}
      metrics
      onScroll={_ => ()}
      onDimensionsChanged={_ => ()}
      onCursorChange={_ => ()}
      bufferHighlights={thousandLineState.bufferHighlights}
      bufferSyntaxHighlights={thousandLineState.bufferSyntaxHighlights}
      diagnostics={thousandLineState.diagnostics}
      completions={thousandLineState.completions}
      tokenTheme={thousandLineState.tokenTheme}
      definition={thousandLineState.definition}
      mode={thousandLineState.mode}
      theme={thousandLineState.theme}
      editorFont={thousandLineState.editorFont}
    />,
  )
  |> ignore;

  rootNode;
};

let editorSurfaceThousandLineLayout = rootNode => {
  Layout.layout(~force=true, rootNode);
};

let options = Reperf.Options.create(~iterations=100, ());

bench(
  ~name="EditorSurface - Rendering: Minimal state",
  ~options,
  ~setup,
  ~f=editorSurfaceMinimalState,
  (),
);

bench(
  ~name="EditorSurface - Rendering: 1000 Lines state",
  ~options,
  ~setup,
  ~f=editorSurfaceThousandLineState,
  (),
);

bench(
  ~name="EditorSurface - Rendering: 1000 Lines state (Indented)",
  ~options,
  ~setup,
  ~f=editorSurfaceThousandLineStateWithIndents,
  (),
);

bench(
  ~name="EditorSurface - Rendering: 100000 Lines state",
  ~options,
  ~setup,
  ~f=editorSurfaceHundredThousandLineState,
  (),
);

/*
 * Bug: This benchmark fails on Linux CI currently
 */
if (Revery.Environment.os == Windows || Revery.Environment.os === Mac) {
  bench(
    ~name="EditorSurface - Layout: 1000 Lines state",
    ~options,
    ~setup=setupSurfaceThousandLineLayout,
    ~f=editorSurfaceThousandLineLayout,
    (),
  );
};
