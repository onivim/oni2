open TestFramework;
open Vim;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s): Context.t);

describe("Format", ({test, _}) => {
  test("gg=G", ({expect, _}) => {
    let _ = resetBuffer();

    let bufferId = Vim.Buffer.(getCurrent() |> getId);

    let effects = ref([]);

    let dispose = onEffect(eff => effects := [eff, ...effects^]);

    input("gg");
    input("=");
    input("G");
    let formatEffect = List.hd(effects^);

    expect.int(List.length(effects^)).toBe(1);
    expect.equal(
      formatEffect,
      Effect.Format(
        Buffer({
          formatType: Format.Indentation,
          bufferId,
          adjustCursor: false,
        }),
      ),
    );

    dispose();
  });
  test("gggqG", ({expect, _}) => {
    let _ = resetBuffer();

    let bufferId = Vim.Buffer.(getCurrent() |> getId);

    let effects = ref([]);

    let dispose = onEffect(eff => effects := [eff, ...effects^]);

    input("gg");
    input("gq");
    input("G");
    let formatEffect = List.hd(effects^);

    expect.int(List.length(effects^)).toBe(1);
    expect.equal(
      formatEffect,
      Effect.Format(
        Buffer({
          formatType: Format.Formatting,
          bufferId,
          adjustCursor: false,
        }),
      ),
    );

    dispose();
  });
  test("gggwG", ({expect, _}) => {
    let _ = resetBuffer();

    let bufferId = Vim.Buffer.(getCurrent() |> getId);

    let effects = ref([]);

    let dispose = onEffect(eff => effects := [eff, ...effects^]);

    input("gg");
    input("gw");
    input("G");
    let formatEffect = List.hd(effects^);

    expect.int(List.length(effects^)).toBe(1);
    expect.equal(
      formatEffect,
      Effect.Format(
        Buffer({formatType: Format.Formatting, bufferId, adjustCursor: true}),
      ),
    );

    dispose();
  });
});
