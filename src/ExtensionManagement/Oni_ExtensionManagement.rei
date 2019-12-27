let install:
  (~extensionFolder: string, ~extensionPath: string) => Lwt.t(unit);

let uninstall:
  (~extensionFolder: string, ~extensionId: string) => Lwt.t(unit);
