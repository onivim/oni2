open TestFramework;

open Oni_Core.Utility;

describe("Path", ({test, _}) => {
  test("hasTrailingSlash", ({expect}) => {
    let cases = [
      ("", false),
      ("a", false),
      ("a\\", true),
      ("a/", true),
      ("\\", true),
      ("/", true),
    ];

    let runCase = ((path, expectedResult)) => {
      expect.bool(Path.hasTrailingSeparator(path)).toBe(expectedResult);
    };

    List.iter(runCase, cases);
  });

  test("trimTrailingSeparator", ({expect}) => {
    let cases = [
      ("", ""),
      ("\\", ""),
      ("/", ""),
      ("a/", "a"),
      ("b\\", "b"),
      ("a/b/", "a/b"),
      ("a\\b\\", "a\\b\\"),
    ];

    let runCase = ((path, expectedResult)) => {
      expect.string(Path.trimTrailingSeparator(path)).toEqual(
        expectedResult,
      );
    };

    List.iter(runCase, cases);
  });

  test("toRelative", ({expect}) => {
    // POSIX-style paths
    expect.string(
      Path.toRelative(~base="/Applications", "/Applications/Onivim2.app"),
    ).
      toEqual(
      "Onivim2.app",
    );

    expect.string(
      Path.toRelative(~base="/Applications/", "/Applications/Onivim2.app"),
    ).
      toEqual(
      "Onivim2.app",
    );

    expect.string(Path.toRelative(~base="/", "/Applications/Onivim2.app")).
      toEqual(
      "Applications/Onivim2.app",
    );

    // Windows-style paths

    expect.string(Path.toRelative(~base="D:", "D:\\Onivim2")).toEqual(
      "Onivim2",
    );
    expect.string(Path.toRelative(~base="D:\\", "D:\\Onivim2")).toEqual(
      "Onivim2",
    );
  });
});
