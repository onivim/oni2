open Oni_Core;

module NodeTask = Oni_Extensions.NodeTask;

module Log = (val Log.withNamespace("Oni2.ExtensionManagement"));

let install: (~extensionFolder: string, ~extensionPath: string) => Lwt.t(unit) =
  (~extensionFolder, ~extensionPath) => {
    let setup = Setup.init();
    let extensionName = Rench.Path.filename(extensionPath);

    let absoluteExtensionPath =
      if (Rench.Path.isAbsolute(extensionPath)) {
        extensionPath;
      } else {
        Rench.Path.join(
          Rench.Environment.getWorkingDirectory(),
          extensionPath,
        );
      };

    Log.infof(m =>
      m("Installing extension %s to %s", extensionName, extensionFolder)
    );

    let promise: Lwt.t(unit) =
      NodeTask.run(
        ~name="Install",
        ~setup,
        ~args=[absoluteExtensionPath, extensionFolder],
        "install-extension.js",
      );

    Lwt.on_success(promise, () => {
      Log.infof(m => m("Successfully installed extension: %s", extensionName))
    });

    Lwt.on_failure(promise, _ => {
      Log.errorf(m => m("Unable to install extension: %s", extensionName))
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
