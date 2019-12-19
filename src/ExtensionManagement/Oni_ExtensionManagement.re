open Oni_Core;

module NodeTask = Oni_Extensions.NodeTask;

module Log = (val Log.withNamespace("Oni2.ExtensionManagement"));

let install: (~extensionFolder: string, ~extensionPath: string) => Lwt.t(unit) =
  (~extensionFolder, ~extensionPath) => {
    let setup = Setup.init();

    Log.infof(m =>
      m("Installing extension %s to %s", extensionPath, extensionFolder)
    );

    let promise: Lwt.t(unit) =
      NodeTask.run(
        ~name="Install",
        ~setup,
        ~args=[extensionPath, "/Users/bryphe/test-extensions"],
        "install-extension.js",
      );

    Lwt.on_success(promise, () => {
      Log.infof(m => m("Successfully installed extension: %s", extensionPath))
    });

    Lwt.on_failure(promise, _ => {
      Log.errorf(m => m("Unable to install extension: %s", extensionPath))
    });
    promise;
  };

let uninstall: (~extensionFolder: string, ~extensionId: string) => Lwt.t(unit) =
  (~extensionFolder, ~extensionId) => {
    // TODO: Implement this
    ignore(extensionFolder);
    ignore(extensionId);
    Lwt.return();
  };
