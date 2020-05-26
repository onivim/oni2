open TestFramework;
open Vim;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s): Context.t);

describe("Goto", ({test, _}) => {
  test("gd", ({expect, _}) => {
    let _ = resetBuffer();

    let updates = ref([]);

    let dispose =
      onGoto((_location, gotoType) => updates := [gotoType, ...updates^]);

    input("gd");
    let gotoType = List.hd(updates^);

    expect.int(List.length(updates^)).toBe(1);
    expect.equal(gotoType, Types.Definition);

    dispose();
  });
  test("gD", ({expect, _}) => {
    let _ = resetBuffer();

    let updates = ref([]);

    let dispose =
      onGoto((_location, gotoType) => updates := [gotoType, ...updates^]);

    input("gD");
    let gotoType = List.hd(updates^);

    expect.int(List.length(updates^)).toBe(1);
    expect.equal(gotoType, Types.Declaration);

    dispose();
  });
});
