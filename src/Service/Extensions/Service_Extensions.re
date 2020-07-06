open Oni_Core;
open Oni_Core.Utility;

module Log = (val Log.withNamespace("Service.Extensions"));

module Constants = {
  let baseUrl = "https://open-vsx.org/api";
};

module Url = {
  let extensionInfo = (publisher, id) => {
    Printf.sprintf("%s/%s/%s", Constants.baseUrl, publisher, id);
  };

  let search = (~offset, ~query) => {
    Printf.sprintf(
      "%s/-/search?query=%s&offset=%d",
      Constants.baseUrl,
      query,
      offset,
    );
  };
};

module Catalog = {
  module Identifier = {
    type t = {
      publisher: string,
      name: string,
    };

    let fromString = str => {
      let items = String.split_on_char('.', str);
      switch (items) {
      | [publisher, name] => Some({publisher, name})
      | _ => None
      };
    };

    let toString = ({publisher, name}) =>
      String.concat("", [publisher, ".", name]);
  };
  module VersionInfo = {
    type t = {
      version: string,
      url: string,
    };

    let decode =
      Json.Decode.(
        key_value_pairs(string)
        |> map(List.map(((version, url)) => {version, url}))
      );

    let toString = ({version, url}) =>
      Printf.sprintf(" - Version %s: %s", version, url);
  };

  module Details = {
    type t = {
      downloadUrl: string,
      repositoryUrl: string,
      homepageUrl: string,
      manifestUrl: string,
      iconUrl: string,
      readmeUrl: string,
      licenseName: string,
      //      licenseUrl: string,
      name: string,
      namespace: string,
      //      downloadCount: int,
      displayName: string,
      description: string,
      //      categories: list(string),
      version: string,
      versions: list(VersionInfo.t),
    };

    let toString =
        ({downloadUrl, displayName, description, homepageUrl, versions, _}) => {
      let versions =
        versions |> List.map(VersionInfo.toString) |> String.concat("\n");
      Printf.sprintf(
        {|Extension %s:
- Description: %s
- Homepage: %s
- Download Url: %s
- Versions:
%s
      |},
        displayName,
        description,
        homepageUrl,
        downloadUrl,
        versions,
      );
    };

    module Decode = {
      open Json.Decode;

      let files = (name, decoder) => field("files", field(name, decoder));
      let downloadUrl = files("download", string);
      let manifestUrl = files("manifest", string);
      let iconUrl = files("icon", string);
      let readmeUrl = files("readme", string);
      let homepageUrl = field("publishedBy", field("homepage", string));

      let decode =
        obj(({field, whatever, _}) =>
          {
            downloadUrl: whatever(downloadUrl),
            manifestUrl: whatever(manifestUrl),
            iconUrl: whatever(iconUrl),
            readmeUrl: whatever(readmeUrl),
            repositoryUrl: field.required("repository", string),
            homepageUrl: whatever(homepageUrl),
            licenseName: field.required("license", string),
            displayName: field.required("displayName", string),
            description: field.required("description", string),
            name: field.required("name", string),
            namespace: field.required("namespace", string),
            version: field.required("version", string),
            versions:
              field.withDefault("allVersions", [], VersionInfo.decode),
          }
        );
    };
  };

  let details = (~setup, {publisher, name}: Identifier.t) => {
    let url = Url.extensionInfo(publisher, name);
    Service_Net.Request.json(~setup, ~decoder=Details.Decode.decode, url);
  };

  module Summary = {
    type t = {
      url: string,
      downloadUrl: string,
      iconUrl: option(string),
      version: string,
      name: string,
      namespace: string,
      displayName: string,
      description: string,
    };

    let decode = {
      open Json.Decode;

      let downloadUrl = field("files", field("download", string));
      let iconUrl = field("files", field("icon", nullable(string)));

      obj(({field, whatever, _}) =>
        {
          url: field.required("url", string),
          downloadUrl: whatever(downloadUrl),
          iconUrl: whatever(iconUrl),
          version: field.required("version", string),
          name: field.required("name", string),
          namespace: field.required("namespace", string),
          displayName: field.required("displayName", string),
          description: field.required("description", string),
        }
      );
    };

    let toString =
        ({displayName, description, version, url, namespace, name, _}) => {
      Printf.sprintf(
        {|%s.%s:
- Name: %s
- Description: %s
- Url: %s
- Version: %s
      |},
        namespace,
        name,
        displayName,
        description,
        url,
        version,
      );
    };
  };

  module SearchResponse = {
    type t = {
      offset: int,
      totalSize: int,
      extensions: list(Summary.t),
    };

    let decode = {
      Json.Decode.(
        obj(({field, _}) =>
          {
            offset: field.required("offset", int),
            totalSize: field.required("totalSize", int),
            extensions:
              field.withDefault("extensions", [], list(Summary.decode)),
          }
        )
      );
    };

    let toString = ({offset, totalSize, extensions}) => {
      let extensionCount = List.length(extensions);

      let extensionStrings =
        extensions |> List.map(Summary.toString) |> String.concat("---\n");
      Printf.sprintf(
        "Showing extensions %d - %d of %d\n%s",
        offset,
        offset + extensionCount,
        totalSize,
        extensionStrings,
      );
    };
  };

  let search = (~offset, ~setup, query) => {
    let url = Url.search(~query, ~offset);
    Service_Net.Request.json(~setup, ~decoder=SearchResponse.decode, url);
  };
};

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
