open Oni_Core;
open Oni_Core.Utility;

module Log = (val Log.withNamespace("Service.Extensions"));

module Catalog = Catalog;
module Query = Query;
module Sub = Sub;

module Internal = {
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

  let installByPath = (~setup, ~extensionsFolder, ~folderName, path) => {
    switch (
      getUserExtensionsDirectory(~overriddenExtensionsDir=extensionsFolder)
    ) {
    | None => Lwt.fail_with("Unable to get extensions folder.")
    | Some(extensionsFolder) =>
      let name = Rench.Path.filename(path);
      let absolutePath =
        if (Rench.Path.isAbsolute(path)) {
          path;
        } else {
          Rench.Path.join(Rench.Environment.getWorkingDirectory(), path);
        };

      Log.debugf(m =>
        m("Installing extension %s to %s", name, extensionsFolder)
      );

      NodeTask.run(
        ~name="Install",
        ~setup,
        ~args=[absolutePath, extensionsFolder, folderName],
        "install-extension.js",
      );
    };
  };
};

module Management = {
  let install = (~setup, ~extensionsFolder=?, path) => {
    // We assume if the requested extension ends with '.vsix',
    // it must be a path.
    let promise =
      if (StringEx.endsWith(~postfix=".vsix", path) && Sys.file_exists(path)) {
        let folderName = Rench.Path.filename(path);
        Internal.installByPath(~setup, ~extensionsFolder, ~folderName, path);
      } else {
        // ...otherwise, query the extension store, download, and install
        Catalog.Identifier.fromString(path)
        |> LwtEx.fromOption(~errorMsg="Invalid extension id: " ++ path)
        |> LwtEx.flatMap(Catalog.details(~setup))
        |> LwtEx.flatMap(
             (
               {downloadUrl, displayName, name, namespace, version, _}: Catalog.Details.t,
             ) => {
             Log.infof(m =>
               m("Downloading %s from %s", displayName, downloadUrl)
             );
             Service_Net.Request.download(~setup, downloadUrl)
             |> Lwt.map(downloadPath => {
                  let folderName =
                    Printf.sprintf("%s.%s-%s", namespace, name, version);

                  (downloadPath, folderName);
                });
           })
        |> LwtEx.flatMap(((downloadPath, folderName)) => {
             Log.infof(m => m("Downloaded successfully to %s", downloadPath));
             Internal.installByPath(
               ~setup,
               ~extensionsFolder,
               ~folderName,
               downloadPath,
             );
           });
      };

    Lwt.on_success(promise, _ => {
      Log.debugf(m => m("Successfully installed extension: %s", path))
    });

    Lwt.on_failure(promise, _ => {
      Log.errorf(m => m("Unable to install extension: %s", path))
    });
    promise;
  };

  let uninstall = (~extensionsFolder=?, extensionId) => {
    open Exthost.Extension;

    let extensions =
      Internal.getUserExtensions(~overriddenExtensionsDir=extensionsFolder);

    let matchingExtensions =
      extensions
      |> List.map((ext: Scanner.ScanResult.t) => {
           (
             ext.manifest |> Manifest.identifier |> String.lowercase_ascii,
             ext.path,
           )
         })
      |> List.filter(((id, _)) =>
           String.equal(extensionId |> String.lowercase_ascii, id)
         );

    if (List.length(matchingExtensions) == 0) {
      Log.info("No matching extension found for: " ++ extensionId);
      Lwt.fail_with("No matching extension found.");
    } else {
      let (_, path) = List.hd(matchingExtensions);

      Log.info("Found matching extension at: " ++ path);

      let promise = Service_OS.Api.rmdir(path);

      Lwt.on_success(promise, _ => {
        Log.debugf(m =>
          m("Successfully uninstalled extension: %s", extensionId)
        )
      });

      Lwt.on_failure(promise, _ => {
        Log.errorf(m => m("Unable to install extension: %s", extensionId))
      });

      promise;
    };
  };

  let get = (~extensionsFolder=?, ()) => {
    Lwt.return(
      Internal.getUserExtensions(~overriddenExtensionsDir=extensionsFolder),
    );
  };
};
