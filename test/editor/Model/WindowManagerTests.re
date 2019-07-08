open TestFramework;


/* open Helpers; */

/* open Oni_Core.Types; */
module WindowManager = Oni_Model.WindowManager;

open WindowManager;

describe("WindowManagerTests", ({describe, _}) =>
  describe("addSplit", ({test, _}) => {
    test("add vertical split", ({expect}) => {

      let splits = WindowManager.empty;
      let rootId = WindowManager.getRootId(splits);

      expect.bool(splits == Parent(Vertical, rootId, [])).toBe(true);

      let split = createSplit(~editorGroupId=1,());

      let splits = addSplit(rootId, Vertical, split, splits);
      
      expect.bool(splits ==Parent(Vertical, rootId, [
        Leaf(split)
      ])).toBe(true);

      let split2 = createSplit(~editorGroupId=2, ());

      let splits = addSplit(rootId, Vertical, split2, splits);
      expect.bool(splits ==Parent(Vertical, rootId, [
        Leaf(split2),
        Leaf(split),
      ])).toBe(true);
    });

    test("parent split changes direction if needed", ({expect}) => {
      let splits = WindowManager.empty;
      let rootId = WindowManager.getRootId(splits);

      expect.bool(splits == Parent(Vertical, rootId, [])).toBe(true);

      let split = createSplit(~editorGroupId=1,());

      let splits = addSplit(rootId, Horizontal, split, splits);
      
      expect.bool(splits ==Parent(Horizontal, rootId, [
        Leaf(split)
      ])).toBe(true);
    });

    })
);
