type t('a) = result('a, string);

let copy: (string, string) => t(unit);

let isDir: Unix.stats => t(unit);

let hasOwner: (int, Unix.stats) => t(unit);

let hasGroup: (int, Unix.stats) => t(unit);

let hasPerm: (int, Unix.stats) => t(unit);

let getUserDataDirectory: unit => t(string);

let getGroupId: unit => t(int);

let getUserId: unit => t(int);

let stat: string => t(option(Unix.stats));

let chown: (string, int, int) => t(unit);

let chmod: (string, ~perm: int=?, unit) => t(unit);

let mkdir: (string, ~perm: int=?, unit) => t(unit);

let mkTempDir: (~prefix: string=?, unit) => string;

let rmdir: string => t(unit);

let getUserDataDirectoryExn: unit => string;

let getOniDirectory: string => t(string);

let getExtensionsFolder: unit => t(string);

let getStoreFolder: unit => t(string);

let getOrCreateConfigFolder: string => t(string);

let getOrCreateConfigFile: (~overridePath: string=?, string) => t(string);
