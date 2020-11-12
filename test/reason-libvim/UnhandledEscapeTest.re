open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let key = s => ignore(Vim.key(s));

describe("onUnhandledEscape", ({test, _}) => {
  test("unhandled escape called with no pending operator", ({expect, _}) => {
    let _ = resetBuffer();

    let callCount = ref(0);
    let dispose = Vim.onUnhandledEscape(() => incr(callCount));

    let _ = Vim.key("<esc>");
    expect.int(callCount^).toBe(1);

    dispose();
  });
  test("unhandled escape not called when in insert", ({expect, _}) => {
    let _ = resetBuffer();

    let callCount = ref(0);
    let dispose = Vim.onUnhandledEscape(() => incr(callCount));

    let _ = Vim.input("i");
    let _ = Vim.key("<esc>");
    expect.int(callCount^).toBe(0);

    dispose();
  });
});
