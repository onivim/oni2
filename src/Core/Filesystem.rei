type t('a) = result('a, string);

let getUserDataDirectory: unit => result(FpExp.t(FpExp.absolute), string);

let getSnippetsFolder: unit => result(FpExp.t(FpExp.absolute), string);

let getExtensionsFolder: unit => result(FpExp.t(FpExp.absolute), string);

let getStoreFolder: unit => result(FpExp.t(FpExp.absolute), string);

let getGlobalStorageFolder: unit => result(FpExp.t(FpExp.absolute), string);

let getWorkspaceStorageFolder:
  unit => result(FpExp.t(FpExp.absolute), string);

let getOrCreateConfigFolder:
  FpExp.t(FpExp.absolute) => result(FpExp.t(FpExp.absolute), string);

let getOrCreateConfigFile:
  (~overridePath: FpExp.t(FpExp.absolute)=?, string) =>
  result(FpExp.t(FpExp.absolute), string);
