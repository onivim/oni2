open Oni_Model;
open TestFramework;

let createEditorBuffer = (~id) => {
  Oni_Core.Buffer.ofLines(~id, [||]) |> Feature_Editor.EditorBuffer.ofBuffer;
};

describe("EditorGroups", ({describe, _}) => {
  describe("setActiveEditor", ({test, _}) => {
    test("should work between editor groups", ({expect, _}) => {
      let editorGroups = EditorGroups.create();
      let defaultFont = Service_Font.default;

      let buffer1 = createEditorBuffer(~id=1);
      let buffer2 = createEditorBuffer(~id=2);

      let (editorGroup1, editor1Id) =
        EditorGroup.create()
        |> EditorGroup.getOrCreateEditorForBuffer(
             ~font=defaultFont,
             ~buffer=buffer1,
           );

      let (editorGroup2, editor2Id) =
        EditorGroup.create()
        |> EditorGroup.getOrCreateEditorForBuffer(
             ~font=defaultFont,
             ~buffer=buffer2,
           );

      let editorGroups =
        editorGroups
        |> EditorGroups.add(~defaultFont, editorGroup1)
        |> EditorGroups.add(~defaultFont, editorGroup2)
        |> EditorGroups.setActiveEditor(~editorId=editor1Id);

      // Verify first group is active
      expect.int(EditorGroups.activeGroupId(editorGroups)).toBe(
        editorGroup1.editorGroupId,
      );

      // Switch to editor2
      // EditorGroup2 should now be the active one!
      let editorGroups =
        editorGroups |> EditorGroups.setActiveEditor(~editorId=editor2Id);

      expect.int(EditorGroups.activeGroupId(editorGroups)).toBe(
        editorGroup2.editorGroupId,
      );
    })
  })
});
