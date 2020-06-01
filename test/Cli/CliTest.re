open TestFramework;

describe("CLI", ({describe, test, _}) => {
  test("no files, no folders", ({expect, _}) => {
    let (options, eff) = Oni_CLI.parse([|"Oni2_editor"|]);
    expect.equal(eff, Run);
    expect.equal(options.folder, None);
    expect.equal(options.filesToOpen, []);
  });
  describe("folder tests", ({test, _}) => {
    test(". should be a folder", ({expect, _}) => {
      let (options, eff) = Oni_CLI.parse([|"Oni2_editor", "."|]);
      expect.equal(eff, Run);
      expect.equal(options.folder != None, true);
      expect.equal(options.filesToOpen, []);
    });

    test(".. should be a folder", ({expect, _}) => {
      let (options, eff) = Oni_CLI.parse([|"Oni2_editor", ".."|]);
      expect.equal(eff, Run);
      expect.equal(options.folder != None, true);
      expect.equal(options.filesToOpen, []);
    });
  });
  describe("file tests", ({test, _}) => {
    // NOTE: This test relies on the working directory being the repo root
    test("README.md should be a file", ({expect, _}) => {
      let (options, eff) = Oni_CLI.parse([|"Oni2_editor", "README.md"|]);
      expect.equal(eff, Run);
      expect.equal(options.filesToOpen |> List.length, 1);
    })
  });
});
