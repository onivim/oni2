open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("onUnhandledEscape", ({test, _}) => {
  test("unhandled escape called with no pending operator", ({expect, _}) => {
    let _ = resetBuffer();

    let callCount = ref(0);
    let dispose = Vim.onUnhandledEscape(() => incr(callCount));

    let _ = Vim.input("<esc>");
    expect.int(callCount^).toBe(1);

    dispose();
  });
  test("unhandled escape not called when in insert", ({expect, _}) => {
    let _ = resetBuffer();

    let callCount = ref(0);
    let dispose = Vim.onUnhandledEscape(() => incr(callCount));

    let _ = Vim.input("i");
    let _ = Vim.input("<esc>");
    expect.int(callCount^).toBe(0);

    dispose();
  });
});
