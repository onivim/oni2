open TestFramework;

open Vim;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");

describe("Mapping / Unmapping", ({describe, _}) => {
  describe("map", ({test, _}) => {
    test("inoremap", ({expect, _}) => {
      let _ = resetBuffer();

      // Force selection mode - and type 'a'
      let (_context, effects) = Vim.command("inoremap jj <esc>");

      expect.equal(
        effects,
        [
          Map(
            Mapping.{
              mode: Insert,
              fromKeys: "jj",
              toValue: "<esc>",
              expression: false,
              recursive: false,
              silent: false,
              scriptId: Mapping.ScriptId.default,
            },
          ),
        ],
      );
    });

    test("map", ({expect, _}) => {
      let _ = resetBuffer();

      // Force selection mode - and type 'a'
      let (_context, effects) = Vim.command("map jj <esc>");

      expect.equal(
        effects,
        [
          Map(
            Mapping.{
              mode: All,
              fromKeys: "jj",
              toValue: "<esc>",
              expression: false,
              recursive: true,
              silent: false,
              scriptId: Mapping.ScriptId.default,
            },
          ),
        ],
      );
    });
  });
  describe("unmap", ({test, _}) => {
    test("simple unmap", ({expect, _}) => {
      let (_context, _effects: list(Effect.t)) =
        Vim.command("inoremap jj <esc>");

      let (_context, effects) = Vim.command("unmap jj");
      expect.equal(effects, [Unmap({mode: All, keys: Some("jj")})]);
    })
  });

  describe("mapclear", ({test, _}) => {
    test("mapclear", ({expect, _}) => {
      let (_context, effects) = Vim.command("mapclear");

      expect.equal(effects, [Unmap({mode: All, keys: None})]);
    })
  });
});
