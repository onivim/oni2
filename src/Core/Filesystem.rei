type t('a) = result('a, string);

let getUserDataDirectory: unit => t(string);

let getExtensionsFolder: unit => t(string);

let getStoreFolder: unit => t(string);

let getGlobalStorageFolder: unit => t(string);

let getOrCreateConfigFolder: string => t(string);

let getOrCreateConfigFile: (~overridePath: string=?, string) => t(string);
