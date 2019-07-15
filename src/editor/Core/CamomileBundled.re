/*
 * CamomileBundled.re
 *
 * 
 */

let setup = Setup.init();

module LocalConfig = {
  let datadir = "";
  let localedir = "";
  let charmapdir = ""; 
  let unimapdir = "";
};

print_endline ("CamomileLibrary.Make");

module Camomile = CamomileLibrary.Make(LocalConfig);
