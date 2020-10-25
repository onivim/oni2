type t('a) = result('a, string);

let getUserDataDirectory: unit => result(Fp.t(Fp.absolute), string);

let getExtensionsFolder: unit => result(Fp.t(Fp.absolute), string);

let getStoreFolder: unit => result(Fp.t(Fp.absolute), string);

let getGlobalStorageFolder: unit => result(Fp.t(Fp.absolute), string);

let getOrCreateConfigFolder:
  Fp.t(Fp.absolute) => result(Fp.t(Fp.absolute), string);

let getOrCreateConfigFile:
  (~overridePath: Fp.t(Fp.absolute)=?, string) =>
  result(Fp.t(Fp.absolute), string);
