/*
 * Utility.re
 *
 * Various store-level utilities
 */

open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2.Store.Utility"));

let getUserExtensionsDirectory = (~overriddenExtensionsDir) => {
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

let getUserExtensions = (~overriddenExtensionsDir) => {
  getUserExtensionsDirectory(~overriddenExtensionsDir)
  |> Option.map(
       FunEx.tap(p =>
         Log.infof(m => m("Searching for user extensions in: %s", p))
       ),
     )
  |> Option.map(Exthost.Extension.Scanner.scan(~category=User))
  |> Option.value(~default=[]);
};
