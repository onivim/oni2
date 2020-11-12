open TestFramework;
open Vim;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s));
let key = s => ignore(Vim.key(s));

describe("DirectoryChanged", ({test, _}) => {
  test("get changed directory event", ({expect, _}) => {
    let _ = resetBuffer();

    let updates: ref(list(string)) = ref([]);
    let dispose = onDirectoryChanged(upd => updates := [upd, ...updates^]);

    ignore(command("cd test"): (Context.t, list(Effect.t)));
    expect.int(List.length(updates^)).toBe(1);

    ignore(command("cd .."): (Context.t, list(Effect.t)));
    expect.int(List.length(updates^)).toBe(2);

    dispose();
  });
  test("change directory via input", ({expect, _}) => {
    let _ = resetBuffer();

    let updates: ref(list(string)) = ref([]);
    let dispose = onDirectoryChanged(upd => updates := [upd, ...updates^]);

    input(":cd test");
    key("<cr>");
    expect.int(List.length(updates^)).toBe(1);

    input(":cd ..");
    key("<cr>");
    expect.int(List.length(updates^)).toBe(2);

    dispose();
  });
});
