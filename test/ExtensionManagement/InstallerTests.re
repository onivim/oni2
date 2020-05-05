open Oni_Core;
open Utility;
open Exthost.Extension;

module ExtM = Oni_ExtensionManagement;

open TestFramework;

let testCollateralPath =
  Rench.Environment.getWorkingDirectory() ++ "/test/collateral/";

let markdownExtension = testCollateralPath ++ "markdown-1.0.0.vsix";

let createExtensionsFolder = () =>
  Filesystem.mkTempDir(~prefix="extensions-test", ());

describe("Installer", ({test, _}) => {
  test("simple install test", ({expect, _}) => {
    let extensionsFolder = createExtensionsFolder();
    let startExtensions =
      Scanner.scan(~category=Development, extensionsFolder);

    expect.equal(List.length(startExtensions), 0);

    let result =
      ExtM.install(~extensionsFolder, ~path=markdownExtension) |> LwtEx.sync;

    expect.equal(result, Ok());

    let afterInstallExtensions =
      Scanner.scan(~category=Development, extensionsFolder);
    expect.equal(List.length(afterInstallExtensions), 1);
  })
});
