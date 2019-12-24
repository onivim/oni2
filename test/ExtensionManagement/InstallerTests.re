open Oni_Core;
open Oni_Extensions;
module ExtM = Oni_ExtensionManagement;

open TestFramework;

let testCollateralPath =
  Rench.Environment.getWorkingDirectory() ++ "/test/collateral/";

let markdownExtension = testCollateralPath ++ "markdown-1.0.0.vsix";

let createExtensionsDirectory = () =>
  Filesystem.mkTempDir(~prefix="extensions-test", ());
describe("Installer", ({test, _}) => {
  test("simple install test", ({expect, _}) => {
    let extensionFolder = createExtensionsDirectory();
    let startExtensions =
      ExtensionScanner.scan(~category=Development, extensionFolder);

    expect.equal(List.length(startExtensions), 0);

    let result =
      ExtM.install(~extensionFolder, ~extensionPath=markdownExtension)
      |> Utility.LwtUtil.sync;

    expect.equal(result, Ok());

    let afterInstallExtensions =
      ExtensionScanner.scan(~category=Development, extensionFolder);
    expect.equal(List.length(afterInstallExtensions), 1);
  })
});
