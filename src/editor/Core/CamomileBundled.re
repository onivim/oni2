/*
 * CamomileBundled.re
 *
 * 
 */

let setup = Setup.init();

module LocalConfig = {
  let datadir = Filename.concat(setup.camomilePath, "database");
  let localedir = Filename.concat(setup.camomilePath, "locales");
  let charmapdir = Filename.concat(setup.camomilePath, "charmaps");
  let unimapdir = Filename.concat(setup.camomilePath, "mappings");
};

let show = () => {
  "Camomile Runtime Paths:\n\n"
  ++ " Datadir: " ++ LocalConfig.datadir ++ "\n"
  ++ " Localedir: " ++ LocalConfig.localedir ++ "\n"
  ++ " Charmapdir: " ++ LocalConfig.charmapdir ++ "\n"
  ++ " Unimapdir: " ++ LocalConfig.unimapdir ++ "\n"
};

module Camomile = CamomileLibrary.Make(LocalConfig);
