open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;
open Feature_Editor;

runTestWithInput(~name="EditorUtf8Test", (input, dispatch, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Types.Normal
  );

  let testFile = getAssetPath("utf8.txt");
  dispatch(Actions.OpenFileByPath(testFile, None, None));

  wait(~name="Verify buffer is loaded", (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> OptionEx.flatMap(Buffer.getShortFriendlyName)
    |> Option.map(name => String.equal(name, "utf8.txt"))
    |> Option.value(~default=false)
  });
  let str =
    "κόσμε"
    |> Oni_Core.BufferLine.make(~indentation=IndentationSettings.default);

  let c0 = BufferLine.getUcharExn(~index=0, str);
  let c1 = BufferLine.getUcharExn(~index=1, str);
  let c2 = BufferLine.getUcharExn(~index=2, str);
  let c3 = BufferLine.getUcharExn(~index=3, str);
  let c4 = BufferLine.getUcharExn(~index=4, str);

  let validateCharacter = (expectedCharacter, state) => {
    state
    |> Selectors.getActiveEditorGroup
    |> Selectors.getActiveEditor
    |> OptionEx.flatMap(Editor.getCharacterUnderCursor)
    |> Option.map(uchar => {Uchar.equal(expectedCharacter, uchar)})
    |> Option.value(~default=false);
  };

  wait(~name="Verify first character", validateCharacter(c0));

  input("l");
  wait(~name="Verify second character", validateCharacter(c1));

  input("l");
  wait(~name="Verify third character", validateCharacter(c2));

  input("l");
  wait(~name="Verify fourth character", validateCharacter(c3));

  input("l");
  wait(~name="Verify fifth character", validateCharacter(c4));
  // TODO: Insert mode validation, too?
  /*dispatch(KeyboardInput("A"));

    wait(~name="Mode switches to insert", (state: State.t) =>
      state.vimMode == Vim.Types.Insert
    );

    wait(~name="Verify fifth character", validateCharacter(c4));
    */
});
