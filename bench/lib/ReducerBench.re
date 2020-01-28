open Oni_Core;
open Oni_Model;
open Oni_Store;
open BenchFramework;

open Helpers;

open Revery.UI;

let rootNode = (new node)();

let setup = () => ();

let doubleEditorSize = () => {
  let state = hundredThousandLineState;

  let _ =
    Reducer.reduce(
      state,
      Actions.EditorGroupSetSize(
        editorGroup.editorGroupId,
        EditorSize.create(~pixelWidth=3200, ~pixelHeight=2400, ()),
      ),
    );
  ();
};

let changeEditorFont = () => {
  let state = hundredThousandLineState;

  let _ =
    Reducer.reduce(
      state,
      Actions.SetEditorFont(
        EditorFont.create(
          ~fontFile="dummy",
          ~fontSize=10.,
          ~measuredWidth=10.,
          ~measuredHeight=10.,
          (),
        ),
      ),
    );
  ();
};

bench(~name="Reducer: SetEditorSize", ~setup, ~f=doubleEditorSize, ());

bench(~name="Reducer: SetEditorFont", ~setup, ~f=doubleEditorSize, ());
