/*
 * Cli.re
 *
 * Helpers for the editor command-line interface
 */

open Oni_CLI;

module Core = Oni_Core;
module ExtC = Service_Extensions.Catalog;
module ExtM = Service_Extensions.Management;
module Log = (val Core.Log.withNamespace("Oni2_editor"));
module ReveryLog = (val Core.Log.withNamespace("Revery"));

module LwtEx = Core.Utility.LwtEx;
module OptionEx = Core.Utility.OptionEx;

let doRemoteCommand = (~pipe, ~files, ~folderToOpen) => {
  let result =
    Feature_ClientServer.Client.openFiles(~server=pipe, ~files, ~folderToOpen)
    |> LwtEx.sync;

  switch (result) {
  | Ok () => 0
  | Error(msg) =>
    prerr_endline("Error executing command: " ++ Printexc.to_string(msg));
    1;
  };
};

let installExtension =
    (path, Oni_CLI.{overriddenExtensionsDir, proxyServer: proxy, _}) => {
  let setup = Core.Setup.init();
  let result =
    ExtM.install(
      ~proxy,
      ~setup,
      ~extensionsFolder=?overriddenExtensionsDir,
      path,
    )
    |> LwtEx.sync;

  switch (result) {
  | Ok(_) =>
    Printf.printf("Successfully installed extension: %s\n", path);
    0;

  | Error(_) =>
    Printf.printf("Failed to install extension: %s\n", path);

    let candidates: result(Service_Extensions.Catalog.SearchResponse.t, exn) =
      ExtC.search(~proxy, ~offset=0, ~setup, path) |> LwtEx.sync;

    switch (candidates) {
    | Ok({extensions, _}) when extensions == [] => ()
    | Ok({extensions, _}) =>
      Printf.printf("\nDid you mean one of these extensions?\n");
      extensions
      |> List.iter((summary: ExtC.Summary.t) => {
           print_endline(
             Printf.sprintf("- %s.%s", summary.namespace, summary.name),
           )
         });
      ();
    | Error(exn) =>
      prerr_endline("Error querying catalog: " ++ Printexc.to_string(exn))
    };
    1;
  };
};

let uninstallExtension = (extensionId, {overriddenExtensionsDir, _}) => {
  let result =
    ExtM.uninstall(~extensionsFolder=?overriddenExtensionsDir, extensionId)
    |> LwtEx.sync;

  switch (result) {
  | Ok(_) =>
    Printf.sprintf("Successfully uninstalled extension: %s\n", extensionId)
    |> print_endline;
    0;

  | Error(msg) =>
    Printf.sprintf(
      "Failed to uninstall extension: %s\n%s",
      extensionId,
      Printexc.to_string(msg),
    )
    |> prerr_endline;
    1;
  };
};

let printVersion = () => {
  print_endline("Onivim 2 (" ++ Core.BuildInfo.version ++ ")");
  0;
};

let listDisplays = () => {
  let _ = Sdl2.init();
  let displays = Sdl2.Display.getDisplays();

  print_endline("Displays:");

  displays
  |> List.iteri((idx, display) => {
       print_endline(
         Printf.sprintf(
           "%d: %s - %s",
           idx,
           Sdl2.Display.getName(display),
           Sdl2.Display.getBounds(display) |> Sdl2.Rect.toString,
         ),
       )
     });

  0;
};

let queryExtension = (extension, {proxyServer: proxy, _}) => {
  let setup = Core.Setup.init();
  Service_Extensions.
    // Try to parse the extension id - either search, or
    // get details
    (
      switch (Catalog.Identifier.fromString(extension)) {
      | Some(identifier) =>
        Catalog.details(~proxy, ~setup, identifier)
        |> LwtEx.sync
        |> (
          fun
          | Ok(ext) => {
              ext |> Catalog.Details.toString |> print_endline;
              0;
            }
          | Error(msg) => {
              prerr_endline(Printexc.to_string(msg));
              1;
            }
        )
      | None =>
        Catalog.search(~proxy, ~offset=0, ~setup, extension)
        |> LwtEx.sync
        |> (
          fun
          | Ok(response) => {
              response |> Catalog.SearchResponse.toString |> print_endline;
              0;
            }
          | Error(msg) => {
              prerr_endline(Printexc.to_string(msg));
              1;
            }
        )
      }
    );
};

let listExtensions = ({overriddenExtensionsDir, _}) => {
  Exthost.Extension.(
    {
      let extensions =
        ExtM.get(~extensionsFolder=?overriddenExtensionsDir, ())
        |> LwtEx.sync
        |> Result.value(~default=[]);

      let printExtension = (ext: Scanner.ScanResult.t) => {
        print_endline(ext.manifest |> Manifest.identifier);
      };
      List.iter(printExtension, extensions);
      0;
    }
  );
};
