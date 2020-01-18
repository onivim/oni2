/*
 * Utility.re
 *
 * Various store-level utilities
 */

open Oni_Core;
open Oni_Extensions;

module FunEx = Oni_Core.Utility.FunEx;
module Option = Oni_Core.Utility.Option;

module Log = (val Log.withNamespace("Oni2.Store.Utility"));

let getUserExtensionsDirectory = (cli: Cli.t) => {
  let overriddenExtensionsDir = cli.overriddenExtensionsDir;

  switch (overriddenExtensionsDir) {
  | Some(p) => Some(p)
  | None =>
    switch (Filesystem.getExtensionsFolder()) {
    | Ok(p) => Some(p)
    | Error(msg) =>
      Log.errorf(m => m("Error discovering user extensions: %s", msg));
      None;
    }
  };
};

let getUserExtensions = (cli: Cli.t) => {
  cli
  |> getUserExtensionsDirectory
  |> Option.map(
       FunEx.tap(p =>
         Log.infof(m => m("Searching for user extensions in: %s", p))
       ),
     )
  |> Option.map(ExtensionScanner.scan(~category=User))
  |> Option.value(~default=[]);
};
