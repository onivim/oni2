let install: (~extensionsFolder: string, ~path: string) => Lwt.t(unit);

let uninstall: (~extensionsFolder: string, ~id: string) => Lwt.t(unit);
