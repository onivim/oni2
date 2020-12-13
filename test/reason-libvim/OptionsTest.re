open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));
let key = s => ignore(Vim.key(s));

describe("Options", ({describe, _}) => {
  describe("effect", ({test, _}) => {
    test(":set minimap", ({expect, _}) => {
      let _ = resetBuffer();

      let effects = ref([]);
      let dispose = onEffect(eff => effects := [eff, ...effects^]);

      let (_: Context.t, _: list(Effect.t)) = Vim.command("set minimap");
      expect.equal(
        effects^,
        [
          SettingChanged(
            Setting.{fullName: "minimap", shortName: None, value: Int(1)},
          ),
        ],
      );

      dispose();
    });
    test(":set rnu", ({expect, _}) => {
      let _ = resetBuffer();

      let effects = ref([]);
      let dispose = onEffect(eff => effects := [eff, ...effects^]);

      let (_: Context.t, _: list(Effect.t)) = Vim.command("set rnu");
      expect.equal(
        effects^,
        [
          SettingChanged(
            Setting.{
              fullName: "relativenumber",
              shortName: Some("rnu"),
              value: Int(1),
            },
          ),
        ],
      );

      dispose();
    });
    test(":set rtp", ({expect, _}) => {
      let _ = resetBuffer();

      let effects = ref([]);
      let dispose = onEffect(eff => effects := [eff, ...effects^]);

      let (_: Context.t, _: list(Effect.t)) = Vim.command("set rtp=abc");
      expect.equal(
        effects^,
        [
          SettingChanged(
            Setting.{
              fullName: "runtimepath",
              shortName: Some("rtp"),
              value: String("abc"),
            },
          ),
        ],
      );

      dispose();
    });
  });
  describe("tabs / spaces", ({test, _}) => {
    test("get / set tab options", ({expect, _}) => {
      let _ = resetBuffer();

      Options.setTabSize(5);
      expect.int(Options.getTabSize()).toBe(5);

      Options.setInsertSpaces(true);
      expect.bool(Options.getInsertSpaces()).toBe(true);

      Options.setTabSize(1);
      expect.int(Options.getTabSize()).toBe(1);

      Options.setInsertSpaces(false);
      expect.bool(Options.getInsertSpaces()).toBe(false);
    });

    test("options persist when switching buffers", ({expect, _}) => {
      let b1 = resetBuffer();

      Options.setTabSize(3);
      Options.setInsertSpaces(true);

      let _ = Buffer.openFile("test/some-random-file.txt");

      Options.setTabSize(4);
      Options.setInsertSpaces(false);

      Buffer.setCurrent(b1);

      expect.int(Options.getTabSize()).toBe(3);
      expect.bool(Options.getInsertSpaces()).toBe(true);
    });

    test("insert spaces / tabs based on settings", ({expect, _}) => {
      let b = resetBuffer();

      Options.setTabSize(3);
      Options.setInsertSpaces(true);

      input("I");
      key("<tab>");

      expect.string(Buffer.getLine(b, LineNumber.zero)).toEqual(
        "   This is the first line of a test file",
      );

      key("<bs>");

      expect.string(Buffer.getLine(b, LineNumber.zero)).toEqual(
        "This is the first line of a test file",
      );

      Options.setTabSize(3);
      Options.setInsertSpaces(false);

      key("<tab>");
      expect.string(Buffer.getLine(b, LineNumber.zero)).toEqual(
        "\tThis is the first line of a test file",
      );
    });
  });
});
