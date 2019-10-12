type t('a) = result('a, string);

let copy: (string, string) => t(unit);

let isDir: Unix.stats => t(unit);

let hasOwner: (int, Unix.stats) => t(unit);

let hasGroup: (int, Unix.stats) => t(unit);

let hasPerm: (int, Unix.stats) => t(unit);

let getHomeDirectory: unit => t(string);

let getGroupId: unit => t(int);

let getUserId: unit => t(int);

let stat: string => t(option(Unix.stats));

let chown: (string, int, int) => t(unit);

let chmod: (string, ~perm: int=?, unit) => t(unit);

let mkdir: (string, ~perm: int=?, unit) => t(unit);

let rmdir: string => t(unit);

let unsafeFindHome: unit => string;

let getOniDirectory: string => t(string);

let getExtensionsFolder: unit => t(string);

let getOrCreateConfigFile: string => t(string);
