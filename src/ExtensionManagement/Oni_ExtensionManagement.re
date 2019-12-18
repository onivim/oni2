open Oni_Core;

let install: (~extensionFolder: string, ~extensionPath: string) => Lwt.t(unit) =
  (~extensionFolder, ~extensionPath) => {
    print_endline("INSTALL");
    Lwt.return();
  };

let uninstall: (~extensionFolder: string, ~extensionId: string) => Lwt.t(unit) =
  (~extensionFolder, ~extensionId) => {
    print_endline("Uninstall");
    Lwt.return();
  };
