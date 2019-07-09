open TestFramework;

/* open Helpers; */

/* open Oni_Core.Types; */
module WindowTree = Oni_Model.WindowTree;
module WindowTreeLayout = Oni_Model.WindowTreeLayout;

open WindowTree;
open WindowTreeLayout;

describe("WindowTreeLayout", ({describe, _}) =>
  describe("layout", ({test, _}) => {
    test("layout vertical splits", ({expect}) => {
      let splits = WindowTree.empty;

      expect.bool(splits == Parent(Vertical, [Empty])).toBe(true);

      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());

      let splits = addSplit(~target=None, Vertical, split1, splits);
      let splits = addSplit(~target=Some(split1.id), Vertical, split2, splits);

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      
      prerr_endline("--");
      List.iter(i => prerr_endline(WindowTreeLayout.show(i)), layoutItems);
      prerr_endline("--");

      expect.bool([
        { id: split2.id, width: 100, height: 200, x: 0, y: 0 },
        { id: split1.id, width: 100, height: 200, x: 100, y: 0 }
      ] == layoutItems).toBe(true);
    });
    
    test("layout horizontal splits", ({expect}) => {
      let splits = WindowTree.empty;

      expect.bool(splits == Parent(Vertical, [Empty])).toBe(true);

      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());

      let splits = addSplit(~target=None, Horizontal, split1, splits);
      let splits = addSplit(~target=Some(split1.id), Horizontal, split2, splits);

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      
      prerr_endline("--");
      List.iter(i => prerr_endline(WindowTreeLayout.show(i)), layoutItems);
      prerr_endline("--");

      expect.bool([
        { id: split2.id, width: 200, height: 100, x: 0, y: 0 },
        { id: split1.id, width: 200, height: 100, x: 0, y: 100 }
      ] == layoutItems).toBe(true);
    });

  })
);
