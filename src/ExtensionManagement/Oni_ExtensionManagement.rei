let install: (~extensionsFolder: string, ~path: string) => Lwt.t(string);

let uninstall: (~extensionsFolder: string, ~id: string) => Lwt.t(unit);
