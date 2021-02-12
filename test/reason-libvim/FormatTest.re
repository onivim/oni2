open EditorCoreTypes;
open TestFramework;
open Vim;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s): (Context.t, list(Effect.t)));

describe("Format", ({test, _}) => {
  test("gg=G", ({expect, _}) => {
    let _ = resetBuffer();

    let bufferId = Vim.Buffer.(getCurrent() |> getId);

    let effects = ref([]);

    let dispose = onEffect(eff => effects := [eff, ...effects^]);

    input("gg");
    input("=");
    input("G");
    let formatEffects =
      effects^
      |> List.filter(
           fun
           | Effect.Format(_) => true
           | _ => false,
         );
    let formatEffect = List.hd(formatEffects);

    expect.int(List.length(formatEffects)).toBe(1);
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
  test("==", ({expect, _}) => {
    let _ = resetBuffer();

    let bufferId = Vim.Buffer.(getCurrent() |> getId);

    let effects = ref([]);

    let dispose = onEffect(eff => effects := [eff, ...effects^]);

    input("==");
    let formatEffect = List.hd(effects^);

    let formatEffects =
      effects^
      |> List.filter(
           fun
           | Effect.Format(_) => true
           | _ => false,
         );
    expect.int(List.length(formatEffects)).toBe(1);
    expect.equal(
      formatEffect,
      Effect.Format(
        Format.Range({
          formatType: Format.Indentation,
          bufferId,
          adjustCursor: false,
          startLine: LineNumber.zero,
          endLine: LineNumber.zero,
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
    let formatEffects =
      effects^
      |> List.filter(
           fun
           | Effect.Format(_) => true
           | _ => false,
         );
    let formatEffect = List.hd(formatEffects);

    expect.int(List.length(formatEffects)).toBe(1);
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
    let formatEffects =
      effects^
      |> List.filter(
           fun
           | Effect.Format(_) => true
           | _ => false,
         );
    let formatEffect = List.hd(formatEffects);

    expect.int(List.length(formatEffects)).toBe(1);
    expect.equal(
      formatEffect,
      Effect.Format(
        Buffer({formatType: Format.Formatting, bufferId, adjustCursor: true}),
      ),
    );

    dispose();
  });
});
