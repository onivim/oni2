open TestFramework;
open Vim;

let reset = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("ColorScheme", ({describe, _}) => {
  let colorSchemeChangedFilter = f =>
    fun
    | Effect.ColorSchemeChanged(maybeScheme) => f(maybeScheme)
    | _ => ();

  describe("set :colorscheme", ({test, _}) => {
    test("empty", ({expect, _}) => {
      let _ = reset();

      let colorSchemeSets = ref([]);
      let dispose =
        onEffect(
          colorSchemeChangedFilter(maybeScheme => {
            colorSchemeSets := [maybeScheme, ...colorSchemeSets^]
          }),
        );

      let _: Context.t = command("colorscheme");

      expect.equal(colorSchemeSets^, [None]);

      dispose();
    });

    test("single word", ({expect, _}) => {
      let _ = reset();

      let colorSchemeSets = ref([]);
      let dispose =
        onEffect(
          colorSchemeChangedFilter(maybeScheme => {
            colorSchemeSets := [maybeScheme, ...colorSchemeSets^]
          }),
        );

      let _: Context.t = command("colorscheme abyss");

      expect.equal(colorSchemeSets^, [Some("abyss")]);

      dispose();
    });
    test("complex word", ({expect, _}) => {
      let _ = reset();

      let colorSchemeSets = ref([]);
      let dispose =
        onEffect(
          colorSchemeChangedFilter(maybeScheme => {
            colorSchemeSets := [maybeScheme, ...colorSchemeSets^]
          }),
        );

      let _: Context.t = command("colorscheme Dark (with parentheses)");

      expect.equal(colorSchemeSets^, [Some("Dark (with parentheses)")]);

      dispose();
    });
  });
});
