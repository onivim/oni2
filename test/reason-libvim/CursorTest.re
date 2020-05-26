open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/reason-libvim/lines_100.txt");
let input = s => ignore(Vim.input(s));

describe("Cursor", ({describe, _}) => {
  describe("setLocation", ({test, _}) => {
    test("cursor location gets updated", ({expect, _}) => {
      let _ = resetBuffer();
      Cursor.setLocation(~line=Index.zero, ~column=Index.(zero + 1));
      expect.int((Cursor.getLine() :> int)).toBe(0);
      expect.int((Cursor.getColumn() :> int)).toBe(1);

      Cursor.setLocation(
        ~line=Index.fromOneBased(3),
        ~column=Index.fromZeroBased(4),
      );
      expect.int((Cursor.getLine() :> int)).toBe(2);
      expect.int((Cursor.getColumn() :> int)).toBe(4);
    });

    test(
      "topline should be updated when moving outside the viewport",
      ({expect, _}) => {
      let _ = resetBuffer();

      let topLineEvents = ref([]);
      let unsubscribe =
        Window.onTopLineChanged(topline =>
          topLineEvents := [topline, ...topLineEvents^]
        );

      Window.setWidth(80);
      Window.setHeight(40);
      Window.setTopLeft(1, 1);

      Cursor.setLocation(~line=Index.zero, ~column=Index.(zero + 1));
      Cursor.setLocation(
        ~line=Index.fromOneBased(90),
        ~column=Index.(zero + 1),
      );

      expect.int(Window.getTopLine()).toBe(61);
      expect.int(List.length(topLineEvents^)).toBe(1);
      expect.int(List.hd(topLineEvents^)).toBe(61);

      unsubscribe();
    });

    test(
      "topline should not be updated when moving inside the viewport",
      ({expect, _}) => {
      Window.setWidth(80);
      Window.setHeight(40);

      let _ = resetBuffer();

      Window.setTopLeft(71, 4);
      Cursor.setLocation(
        ~line=Index.fromOneBased(90),
        ~column=Index.(zero + 1),
      );

      expect.int(Window.getTopLine()).toBe(71);
    });
  });
  describe("normal mode", ({test, _}) => {
    test("j / k", ({expect, _}) => {
      let _ = resetBuffer();

      let cursorMoves: ref(list(Location.t)) = ref([]);
      let dispose = Cursor.onMoved(p => cursorMoves := [p, ...cursorMoves^]);

      expect.int((Cursor.getLine() :> int)).toBe(0);
      expect.int((Cursor.getColumn() :> int)).toBe(0);
      expect.int(List.length(cursorMoves^)).toBe(0);

      input("j");

      expect.int((Cursor.getLine() :> int)).toBe(1);
      expect.int((Cursor.getColumn() :> int)).toBe(0);
      expect.int(List.length(cursorMoves^)).toBe(1);

      input("j");

      expect.int((Cursor.getLine() :> int)).toBe(2);
      expect.int((Cursor.getColumn() :> int)).toBe(0);
      expect.int(List.length(cursorMoves^)).toBe(2);

      input("2");
      input("k");

      expect.int((Cursor.getLine() :> int)).toBe(0);
      expect.int((Cursor.getColumn() :> int)).toBe(0);
      expect.int(List.length(cursorMoves^)).toBe(3);

      dispose();
    });

    test("gg / G", ({expect, _}) => {
      let _ = resetBuffer();

      let cursorMoves: ref(list(Location.t)) = ref([]);
      let dispose = Cursor.onMoved(p => cursorMoves := [p, ...cursorMoves^]);

      expect.int((Cursor.getLine() :> int)).toBe(0);
      expect.int((Cursor.getColumn() :> int)).toBe(0);
      expect.int(List.length(cursorMoves^)).toBe(0);

      input("G");

      expect.int((Cursor.getLine() :> int)).toBe(99);
      expect.int((Cursor.getColumn() :> int)).toBe(0);
      expect.int(List.length(cursorMoves^)).toBe(1);

      input("g");
      input("g");

      expect.int((Cursor.getLine() :> int)).toBe(0);
      expect.int((Cursor.getColumn() :> int)).toBe(0);
      expect.int(List.length(cursorMoves^)).toBe(2);

      dispose();
    });
  });
});
