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

print_endline ("CamomileLibrary.Make");

print_endline ("Datadir: " ++ LocalConfig.datadir);
print_endline ("Localedir: " ++ LocalConfig.localedir);
print_endline ("Charmapdir: " ++ LocalConfig.charmapdir);
print_endline ("Unimapdir: " ++ LocalConfig.unimapdir);

module Camomile = CamomileLibrary.Make(LocalConfig);
