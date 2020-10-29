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

  describe("line comment", ({test, _}) => {
    test("toggle comment based on settings", ({expect, _}) => {
      let b = resetBuffer();

      Options.setLineComment("; ");

      input("g");
      input("c");
      input("c");

      expect.string(Buffer.getLine(b, LineNumber.zero)).toEqual(
        "; This is the first line of a test file",
      );

      input("g");
      input("c");
      input("c");

      expect.string(Buffer.getLine(b, LineNumber.zero)).toEqual(
        "This is the first line of a test file",
      );

      Options.setLineComment("!!");

      input("g");
      input("c");
      input("c");

      expect.string(Buffer.getLine(b, LineNumber.zero)).toEqual(
        "!!This is the first line of a test file",
      );
    })
  });

  describe("all options - sanity test", ({test, _}) => {
    let boolValues = ["0", "1"];
    let cases = [
      ("aleph", Some("al"), boolValues),
      ("antialias", Some("anti"), boolValues),
      ("ambiwidth", Some("ambw"), ["single", "double"]),
      ("autochdir", Some("acd"), boolValues),
      ("autoindent", Some("ai"), boolValues),
      ("autoread", Some("ar"), boolValues),
      ("autowrite", Some("aw"), boolValues),
      ("autowriteall", Some("awa"), boolValues),
      ("background", Some("bg"), ["dark", "light"]),
      ("backspace", Some("bs"), ["indent", "eol", "indent,eol"]),
      ("backup", Some("bk"), boolValues),
      ("backupcopy", Some("bkc"), ["yes", "no", "auto"]),
      ("backupdir", Some("bdir"), ["/some/path"]),
      ("backupext", Some("bdir"), ["~backup~"]),
      ("backupskip", Some("bsk"), ["/tmp/"]),
      // Remove ballooneval
      // Remove ballondelay
      // Remove balloonevalterm
      // Remove FEAT_BEVAL_TERM
      // Remove beautify
      ("belloff", Some("bo"), ["all", "cursor", "all, cursor"]),
      // Set binary to 0 on completion!
      ("binary", Some("bo"), boolValues @ ["0"]),
      // Remove bioskey
      ("bomb", None, boolValues),
      ("breakat", Some("brk"), ["abc"]),
      ("breakindent", Some("bri"), boolValues),
      ("breakindentopt", Some("briopt"), ["min:10,shift:10,sbr"]),
      ("browsedir", Some("bsdir"), ["last"]),
      ("bufhidden", Some("bh"), ["hide", "wipe"]),
      ("buflisted", Some("bl"), boolValues),
      ("buftype", Some("bt"), ["", "acwrite", "quickfix", "terminal"]),
      ("casemap", Some("cmp"), ["internal", "keepascii"]),
      ("cdpath", Some("cd"), ["../", "/abc"]),
      // NEXT: cedit
    ];

    cases
    |> List.iter(((cmd, maybeShortCmd, _values)) => {
         test(
           "Read value: " ++ cmd,
           ({expect, _}) => {
             prerr_endline("Reading command: " ++ cmd);
             let _ = Vim.command(":set " ++ cmd ++ "?");
             expect.bool(true).toBe(true);
           },
         );

         maybeShortCmd
         |> Option.iter(shortCmd => {
              test(
                "Read value: " ++ shortCmd,
                ({expect, _}) => {
                  prerr_endline("Reading short command: " ++ shortCmd);
                  let _ = Vim.command(":set " ++ cmd ++ "?");
                  expect.bool(true).toBe(true);
                },
              )
            });
       });
  });
});
