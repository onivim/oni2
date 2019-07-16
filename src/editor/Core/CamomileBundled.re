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

module Camomile = CamomileLibrary.Make(LocalConfig);
