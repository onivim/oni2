open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("Visual", ({describe, _}) => {
  describe("getRange", ({test, _}) =>
    test("simple range", ({expect, _}) => {
      let _ = resetBuffer();

      input("v");
      input("l");

      let range = Visual.getRange();
      expect.int((range.start.line :> int)).toBe(0);
      expect.int((range.start.column :> int)).toBe(0);
      expect.int((range.stop.line :> int)).toBe(0);
      expect.int((range.stop.column :> int)).toBe(1);

      input("3");
      input("l");

      let range = Visual.getRange();
      expect.int((range.start.line :> int)).toBe(0);
      expect.int((range.start.column :> int)).toBe(0);
      expect.int((range.stop.line :> int)).toBe(0);
      expect.int((range.stop.column :> int)).toBe(4);

      input("j");
      let range = Visual.getRange();
      expect.int((range.start.line :> int)).toBe(0);
      expect.int((range.start.column :> int)).toBe(0);
      expect.int((range.stop.line :> int)).toBe(1);
      expect.int((range.stop.column :> int)).toBe(4);
    })
  );

  describe("getType", ({test, _}) =>
    test("simple range", ({expect, _}) => {
      let _ = resetBuffer();

      let vt = Visual.getType();
      expect.bool(vt == None).toBe(true);

      input("v");
      let vt = Visual.getType();
      expect.bool(vt == Character).toBe(true);
      input("<esc>");

      input("V");
      let vt = Visual.getType();
      expect.bool(vt == Line).toBe(true);
      input("<esc>");

      input("<C-v>");
      let vt = Visual.getType();
      expect.bool(vt == Block).toBe(true);
      input("<esc>");

      let vt = Visual.getType();
      expect.bool(vt == None).toBe(true);
    })
  );

  describe("onRangeChanged", ({test, _}) =>
    test("dispatches on change", ({expect, _}) => {
      let _ = resetBuffer();

      let rangeChanges: ref(list(Range.t)) = ref([]);
      let dispose =
        Visual.onRangeChanged(vr => {
          open Vim.VisualRange;
          let {range, _} = vr;
          rangeChanges := [range, ...rangeChanges^];
        });

      input("V");

      expect.int(List.length(rangeChanges^)).toBe(1);
      let r = List.hd(rangeChanges^);
      expect.int((r.start.line :> int)).toBe(0);
      expect.int((r.start.column :> int)).toBe(0);
      expect.int((r.stop.line :> int)).toBe(0);
      expect.int((r.stop.column :> int)).toBe(0);

      input("j");

      expect.int(List.length(rangeChanges^)).toBe(2);
      let r = List.hd(rangeChanges^);
      expect.int((r.start.line :> int)).toBe(0);
      expect.int((r.start.column :> int)).toBe(0);
      expect.int((r.stop.line :> int)).toBe(1);
      expect.int((r.stop.column :> int)).toBe(0);

      dispose();
    })
  );
});
