open TestFramework;

let noenv = _ => None;

describe("CLI", ({describe, test, _}) => {
  test("no files, no folders", ({expect, _}) => {
    let (options, eff) = Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor"|]);
    expect.equal(eff, Run);
    expect.equal(options.folder, None);
    expect.equal(options.filesToOpen, []);
  });
  describe("folder tests", ({test, _}) => {
    test(". should be a folder", ({expect, _}) => {
      let (options, eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "."|]);
      expect.equal(eff, Run);
      expect.equal(options.folder != None, true);
      expect.equal(options.filesToOpen, []);
    });

    test(".. should be a folder", ({expect, _}) => {
      let (options, eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", ".."|]);
      expect.equal(eff, Run);
      expect.equal(options.folder != None, true);
      expect.equal(options.filesToOpen, []);
    });
  });
  describe("file tests", ({test, _}) => {
    // NOTE: This test relies on the working directory being the repo root
    test("README.md should be a file", ({expect, _}) => {
      let (options, eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "README.md"|]);
      expect.equal(eff, Run);
      expect.equal(options.filesToOpen |> List.length, 1);
    });

    test("#1983: Specifying a file should not open a folder", ({expect, _}) => {
      let (options, eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "some-file.md"|]);
      expect.equal(eff, Run);
      expect.equal(options.filesToOpen |> List.length, 1);
      expect.equal(options.folder, None);
    });
  });
  describe("syntax server", ({test, _}) => {
    test("Syntax server with PID", ({expect, _}) => {
      let (_options, eff) =
        Oni_CLI.parse(
          ~getenv=noenv,
          [|"Oni2_editor", "--syntax-highlight-service", "1234:named-pipe"|],
        );
      expect.equal(
        eff,
        StartSyntaxServer({parentPid: "1234", namedPipe: "named-pipe"}),
      );
    })
  });
  describe("log level", ({test, _}) => {
    test("--debug-exthost should set logExthost", ({expect, _}) => {
      let (options, _eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "--debug-exthost"|]);
      expect.equal(options.logExthost, true);
    });
    test("--silent should still require a console", ({expect, _}) => {
      let (options, _eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "-f", "--silent"|]);
      expect.equal(options.logLevel, None);
      expect.equal(options.needsConsole, true);
    });
    test("--trace should set log level", ({expect, _}) => {
      let (options, _eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "-f", "--trace"|]);
      expect.equal(options.logLevel, Some(Timber.Level.trace));
    });
    test("-f should not override --trace", ({expect, _}) => {
      let (options, _eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "--trace", "-f"|]);
      expect.equal(options.logLevel, Some(Timber.Level.trace));
    });
    test("-f should set info level", ({expect, _}) => {
      let (options, _eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "-f"|]);
      expect.equal(options.logLevel, Some(Timber.Level.info));
    });
    test("no log level set by default", ({expect, _}) => {
      let (options, _eff) = Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor"|]);
      expect.equal(options.logLevel, None);
    });
  });

  describe("vim ex commands", ({test, _}) => {
    test("-c should add ex command", ({expect, _}) => {
      let (options, _eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "-c", "term"|]);

      expect.equal(options.vimExCommands, ["term"]);
    });
    test("multiple -c commands should be handled, in order", ({expect, _}) => {
      let (options, _eff) =
        Oni_CLI.parse(
          ~getenv=noenv,
          [|"Oni2_editor", "-c", "term", "-c", "xa!"|],
        );

      expect.equal(options.vimExCommands, ["term", "xa!"]);
    });
    test("+ anonymous arg should add ex command", ({expect, _}) => {
      let (options, _eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "+term"|]);

      expect.equal(options.vimExCommands, ["term"]);
      expect.equal(options.filesToOpen, []);
      expect.equal(options.folder, None);
    });
    test("multiple + anonymous arg should add ex commands", ({expect, _}) => {
      let (options, _eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "+term", "+xa!"|]);

      expect.equal(options.vimExCommands, ["term", "xa!"]);
      expect.equal(options.filesToOpen, []);
      expect.equal(options.folder, None);
    });
  });
});
