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

      ignore(command("colorscheme"): (Context.t, list(Effect.t)));

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

      ignore(command("colorscheme abyss"): (Context.t, list(Effect.t)));

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

      ignore(
        command("colorscheme Dark (with parentheses)"): (
                                                         Context.t,
                                                         list(Effect.t),
                                                       ),
      );

      expect.equal(colorSchemeSets^, [Some("Dark (with parentheses)")]);

      dispose();
    });
  });
});
