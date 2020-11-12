open TestFramework;
open Vim;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("Yank", ({test, _}) => {
  test("onYank works for deleting single line", ({expect, _}) => {
    let _ = resetBuffer();

    let yanks: ref(list(Yank.t)) = ref([]);
    let dispose = Vim.onYank(yank => yanks := [yank, ...yanks^]);

    let _ = Vim.input("d");
    let _ = Vim.input("d");

    expect.int(List.length(yanks^)).toBe(1);
    let del: Yank.t = List.hd(yanks^);

    expect.int(del.startLine).toBe(1);
    expect.int(del.startColumn).toBe(0);
    expect.int(del.endLine).toBe(1);
    expect.int(del.endColumn).toBe(0);

    expect.bool(del.operator == Yank.Delete).toBe(true);
    expect.int(Array.length(del.lines)).toBe(1);
    expect.string(del.lines[0]).toEqual(
      "This is the first line of a test file",
    );

    dispose();
  });
  test("onYank works for deleting multiple lines", ({expect, _}) => {
    let _ = resetBuffer();

    let yanks: ref(list(Yank.t)) = ref([]);
    let dispose = Vim.onYank(yank => yanks := [yank, ...yanks^]);

    let _ = Vim.input("d");
    let _ = Vim.input("j");

    expect.int(List.length(yanks^)).toBe(1);
    let del: Yank.t = List.hd(yanks^);

    expect.int(Char.code(del.register)).toBe(0);
    expect.bool(del.yankType == Yank.Line).toBe(true);
    expect.bool(del.operator == Yank.Delete).toBe(true);
    expect.int(Array.length(del.lines)).toBe(2);
    expect.string(del.lines[0]).toEqual(
      "This is the first line of a test file",
    );
    expect.string(del.lines[1]).toEqual(
      "This is the second line of a test file",
    );

    dispose();
  });

  test("onYank works for single character", ({expect, _}) => {
    let _ = resetBuffer();

    let yanks: ref(list(Yank.t)) = ref([]);
    let dispose = Vim.onYank(yank => yanks := [yank, ...yanks^]);

    let _ = Vim.input("l");
    let _ = Vim.input("x");

    expect.int(List.length(yanks^)).toBe(1);
    let del: Yank.t = List.hd(yanks^);

    expect.int(del.startLine).toBe(1);
    expect.int(del.startColumn).toBe(1);
    expect.int(del.endLine).toBe(1);
    expect.int(del.endColumn).toBe(2);
    expect.int(Char.code(del.register)).toBe(0);
    expect.bool(del.yankType == Yank.Char).toBe(true);
    expect.bool(del.operator == Yank.Delete).toBe(true);
    expect.int(Array.length(del.lines)).toBe(1);
    expect.string(del.lines[0]).toEqual("h");

    dispose();
  });

  test("onYank works for yanking a line", ({expect, _}) => {
    let _ = resetBuffer();

    let yanks: ref(list(Yank.t)) = ref([]);
    let dispose = Vim.onYank(yank => yanks := [yank, ...yanks^]);

    let _ = Vim.input("y");
    let _ = Vim.input("y");

    expect.int(List.length(yanks^)).toBe(1);
    let del: Yank.t = List.hd(yanks^);

    expect.int(Char.code(del.register)).toBe(0);
    expect.bool(del.yankType == Yank.Line).toBe(true);
    expect.bool(del.operator == Yank.Yank).toBe(true);
    expect.int(Array.length(del.lines)).toBe(1);
    expect.string(del.lines[0]).toEqual(
      "This is the first line of a test file",
    );

    dispose();
  });

  test("onYank sets register correctly", ({expect, _}) => {
    let _ = resetBuffer();

    let yanks: ref(list(Yank.t)) = ref([]);
    let dispose = Vim.onYank(yank => yanks := [yank, ...yanks^]);

    let _ = Vim.input("\"");
    let _ = Vim.input("a");
    let _ = Vim.input("y");
    let _ = Vim.input("y");

    expect.int(List.length(yanks^)).toBe(1);
    let del: Yank.t = List.hd(yanks^);

    expect.int(Char.code(del.register)).toBe(Char.code('a'));
    expect.bool(del.yankType == Yank.Line).toBe(true);
    expect.bool(del.operator == Yank.Yank).toBe(true);
    expect.int(Array.length(del.lines)).toBe(1);
    expect.string(del.lines[0]).toEqual(
      "This is the first line of a test file",
    );

    dispose();
  });
});
