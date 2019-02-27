open Oni_UI;
open BenchFramework;

open Helpers;

open Revery.UI;

let rootNode = (new node)();

let setup = () => ();

let editorSurfaceMinimalState = () => {
  let _ =
    React.RenderedElement.render(
      rootNode,
      <EditorSurface state=simpleState />,
    );
  ();
};

let editorSurfaceThousandLineState = () => {
  let _ =
    React.RenderedElement.render(
      rootNode,
      <EditorSurface state=thousandLineState />,
    );
  ();
};

let editorSurfaceHundredThousandLineState = () => {
  let _ =
    React.RenderedElement.render(
      rootNode,
      <EditorSurface state=hundredThousandLineState />,
    );
  ();
};

let setupSurfaceThousandLineLayout = () => {
  let rootNode = (new viewNode)();

  rootNode#setStyle(
    Style.make(~position=LayoutTypes.Relative, ~width=1600, ~height=1200, ()),
  );

  let container = React.Container.create(rootNode);
  React.Container.update(container, <EditorSurface state=thousandLineState />)
  |> ignore;

  rootNode;
};

let editorSurfaceThousandLineLayout = rootNode => {
  Layout.layout(rootNode, 1.0, 1);
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
