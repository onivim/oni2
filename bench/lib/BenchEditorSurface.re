open Oni_Core;
open Oni_UI;
open BenchFramework;

open Revery.UI;

let rootNode = (new node)();
let simpleState = State.create();

let editorSurfaceMinimalState = () => {
    let _ = React.RenderedElement.render(rootNode, <EditorSurface state={simpleState} />);
};

bench(~name="EditorSurface: Minimal state", ~f=editorSurfaceMinimalState, ());
