open Oni_Core;
open Oni_Extensions;

module LwtEx = Utility.LwtEx;

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
      ExtensionScanner.scan(~category=Development, extensionsFolder);

    expect.equal(List.length(startExtensions), 0);

    let result =
      ExtM.install(~extensionsFolder, ~path=markdownExtension) |> LwtEx.sync;

    expect.equal(result, Ok());

    let afterInstallExtensions =
      ExtensionScanner.scan(~category=Development, extensionsFolder);
    expect.equal(List.length(afterInstallExtensions), 1);
  })
});
