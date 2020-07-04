open Oni_Core;
open Utility;
open Exthost.Extension;

module ExtM = Service_Extensions.Management;

open TestFramework;

let testCollateralPath =
  Rench.Environment.getWorkingDirectory() ++ "/test/collateral/";

let markdownExtension = testCollateralPath ++ "markdown-1.0.0.vsix";

let createExtensionsFolder = () =>
  Filesystem.mkTempDir(~prefix="extensions-test", ());

let setup = Setup.init();

describe("Management", ({describe, _}) => {
  describe("uninstall", ({test, _}) => {
    test("uninstall (from file)", ({expect, _}) => {
      let extensionsFolder = createExtensionsFolder();

      let installResult =
        ExtM.install(~setup, ~extensionsFolder, markdownExtension)
        |> LwtEx.sync;

      expect.equal(Result.is_ok(installResult), true);

      let uninstallResult =
        ExtM.uninstall(~extensionsFolder, "vscode.markdown") |> LwtEx.sync;

      expect.equal(Result.is_ok(uninstallResult), true);

      let afterUninstallExtensions =
        Scanner.scan(~category=Development, extensionsFolder);
      expect.equal(List.length(afterUninstallExtensions), 0);
    })
  });
  describe("install", ({test, _}) => {
    test("install from file test", ({expect, _}) => {
      let extensionsFolder = createExtensionsFolder();
      let startExtensions =
        Scanner.scan(~category=Development, extensionsFolder);

      expect.equal(List.length(startExtensions), 0);

      let result =
        ExtM.install(~setup, ~extensionsFolder, markdownExtension)
        |> LwtEx.sync;

      expect.equal(Result.is_ok(result), true);

      let afterInstallExtensions =
        Scanner.scan(~category=Development, extensionsFolder);
      expect.equal(List.length(afterInstallExtensions), 1);
    });

    test("install by id", ({expect, _}) => {
      let extensionsFolder = createExtensionsFolder();
      let startExtensions =
        Scanner.scan(~category=Development, extensionsFolder);

      expect.equal(List.length(startExtensions), 0);

      let result =
        ExtM.install(~setup, ~extensionsFolder, "redhat.java") |> LwtEx.sync;

      expect.equal(Result.is_ok(result), true);

      let afterInstallExtensions =
        Scanner.scan(~category=Development, extensionsFolder);
      expect.equal(List.length(afterInstallExtensions), 1);
    });
    test("invalid id returns failure", ({expect, _}) => {
      let extensionsFolder = createExtensionsFolder();
      let startExtensions =
        Scanner.scan(~category=Development, extensionsFolder);

      expect.equal(List.length(startExtensions), 0);

      let result =
        ExtM.install(
          ~setup,
          ~extensionsFolder,
          "invalid-namespace.invalid-extension-id",
        )
        |> LwtEx.sync;

      expect.equal(Result.is_error(result), true);

      let afterInstallExtensions =
        Scanner.scan(~category=Development, extensionsFolder);
      expect.equal(List.length(afterInstallExtensions), 0);
    });
  });
});
