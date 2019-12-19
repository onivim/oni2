open Oni_Core;

module NodeTask = Oni_Extensions.NodeTask;

let install: (~extensionFolder: string, ~extensionPath: string) => Lwt.t(unit) =
  (~extensionFolder, ~extensionPath) => {
    let setup = Setup.init();
    
    let promise: Lwt.t(unit) = NodeTask.run(
      ~name="Install",
      ~onMessage=(_) => (),
      ~scheduler=(f) => f(),
      ~script="install-extension.js",
      ~setup,
      ~args=[
        extensionPath, 
        "/Users/bryphe/test-extensions"
      ],
    );
    promise;
  };

let uninstall: (~extensionFolder: string, ~extensionId: string) => Lwt.t(unit) =
  (~extensionFolder, ~extensionId) => {
    print_endline("Uninstall");
    Lwt.return();
  };
