open TestFramework;

/* open Helpers; */

/* open Oni_Core.Types; */
module WindowTree = Oni_Model.WindowTree;
module WindowTreeLayout = Oni_Model.WindowTreeLayout;

open WindowTree;
open WindowTreeLayout;

describe("WindowTreeLayout", ({describe, _}) => {
  describe("layout", ({test, _}) => {
    test("layout vertical splits", ({expect}) => {
      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());

      let splits = WindowTree.empty
      |> addSplit(~target=None, Vertical, split1)
      |> addSplit(~target=Some(split1.id), Vertical, split2);

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.bool([
        { id: split2.id, width: 100, height: 200, x: 0, y: 0 },
        { id: split1.id, width: 100, height: 200, x: 100, y: 0 }
      ] == layoutItems).toBe(true);
    });
    
    test("layout horizontal splits", ({expect}) => {
      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());

      let splits = WindowTree.empty
      |> addSplit(~target=None, Horizontal, split1)
      |> addSplit(~target=Some(split1.id), Horizontal, split2);

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.bool([
        { id: split2.id, width: 200, height: 100, x: 0, y: 0 },
        { id: split1.id, width: 200, height: 100, x: 0, y: 100 }
      ] == layoutItems).toBe(true);
    });
    test("layout mixed splits", ({expect}) => {

        let split1 = createSplit(~editorGroupId=1, ());
        let split2 = createSplit(~editorGroupId=2, ());
        let split3 = createSplit(~editorGroupId=3, ());

        let splits = WindowTree.empty
          |> addSplit(~target=None, Horizontal, split1)
          |> addSplit(~target=Some(split1.id), Horizontal, split2)
          |> addSplit(~target=Some(split1.id), Vertical, split3);

        let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

        List.iter(i => prerr_endline(WindowTreeLayout.show(i)), layoutItems);
        
        expect.bool([
          { id: split2.id, width: 200, height: 100, x: 0, y: 0 },
          { id: split3.id, width: 100, height: 100, x: 0, y: 100 },
          { id: split1.id, width: 100, height: 100, x: 100, y: 100 }
        ] == layoutItems).toBe(true);
    });
  });
});
