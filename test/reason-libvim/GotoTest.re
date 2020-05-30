open TestFramework;
open Vim;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s): Context.t);

describe("Goto", ({test, _}) => {
  test("gd", ({expect, _}) => {
    let _ = resetBuffer();

    let updates = ref([]);

    let dispose =
      onEffect((gotoType) => updates := [gotoType, ...updates^]);

    input("gd");
    let gotoType = List.hd(updates^);

    expect.int(List.length(updates^)).toBe(1);
    expect.equal(gotoType, Effect.Goto(Goto.Definition));

    dispose();
  });
  test("gD", ({expect, _}) => {
    let _ = resetBuffer();

    let updates = ref([]);

    let dispose =
      onEffect((gotoType) => updates := [gotoType, ...updates^]);

    input("gD");
    let gotoType = List.hd(updates^);

    expect.int(List.length(updates^)).toBe(1);
    expect.equal(gotoType, Effect.Goto(Goto.Declaration));

    dispose();
  });
  test("gh", ({expect, _}) => {
    let _ = resetBuffer();

    let updates = ref([]);

    let dispose =
      onEffect((gotoType) => updates := [gotoType, ...updates^]);

    input("gh");
    let gotoType = List.hd(updates^);

    expect.int(List.length(updates^)).toBe(1);
    expect.equal(gotoType, Effect.Goto(Goto.Hover));

    dispose();
  });
});
