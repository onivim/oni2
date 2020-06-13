open TestFramework;
open Vim;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s): Context.t);

describe("Format", ({test, _}) => {
  test("gg=G", ({expect, _}) => {
    let _ = resetBuffer();

    let bufferId = Vim.Buffer.(getCurrent() |> getId);

    let effects = ref([]);

    let dispose = onEffect(gotoType => updates := [gotoType, ...updates^]);

    input("gg=G");
    let formatEffect = List.hd(updates^);

    expect.int(List.length(effects^)).toBe(1);
    expect.equal(formateffect, Effect.Format(Buffer({
      bufferId,
      returnCursor: false,
    });

    dispose();
  });
});
