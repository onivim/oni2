open Oni_Core;

module NodeTask = Oni_Core.NodeTask;

module Log = (val Log.withNamespace("Oni2.Extensions.ExtensionManagement"));

let install = (~extensionsFolder, ~path) => {
  let setup = Setup.init();
  let name = Rench.Path.filename(path);

  let absolutePath =
    if (Rench.Path.isAbsolute(path)) {
      path;
    } else {
      Rench.Path.join(Rench.Environment.getWorkingDirectory(), path);
    };

  Log.debugf(m => m("Installing extension %s to %s", name, extensionsFolder));

  let promise: Lwt.t(unit) =
    NodeTask.run(
      ~name="Install",
      ~setup,
      ~args=[absolutePath, extensionsFolder],
      "install-extension.js",
    );

  Lwt.on_success(promise, () => {
    Log.debugf(m => m("Successfully installed extension: %s", name))
  });

  Lwt.on_failure(promise, _ => {
    Log.errorf(m => m("Unable to install extension: %s", name))
  });
  promise;
};

let uninstall: (~extensionsFolder: string, ~id: string) => Lwt.t(unit) =
  (~extensionsFolder, ~id) => {
    // TODO: Implement this
    ignore(extensionsFolder);
    ignore(id);
    Lwt.return();
  };
