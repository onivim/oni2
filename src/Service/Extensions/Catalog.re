open Oni_Core;

module Log = (val Log.withNamespace("Service.Extensions.Catalog"));

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

module Identifier = {
  [@deriving show]
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
  [@deriving show]
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
  [@deriving show]
  type t = {
    downloadUrl: string,
    repositoryUrl: option(string),
    homepageUrl: string,
    manifestUrl: string,
    iconUrl: option(string),
    readmeUrl: option(string),
    licenseName: option(string),
    //      licenseUrl: string,
    name: string,
    namespace: string,
    //      downloadCount: int,
    displayName: option(string),
    description: option(string),
    //      categories: list(string),
    version: string,
    versions: list(VersionInfo.t),
  };

  let id = ({namespace, name, _}) => {
    namespace ++ "." ++ name;
  };

  let displayName = ({displayName, _} as details) => {
    displayName |> Option.value(~default=id(details));
  };

  let toString =
      ({downloadUrl, description, homepageUrl, versions, _} as details) => {
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
      details |> displayName,
      description |> Option.value(~default="(null)"),
      homepageUrl,
      downloadUrl,
      versions,
    );
  };

  module Decode = {
    open Json.Decode;

    let files = (name, decoder) => field("files", field(name, decoder));
    let filesOpt = (name, decoder) =>
      field("files", field_opt(name, decoder));
    let downloadUrl = files("download", string);
    let manifestUrl = files("manifest", string);
    let iconUrl = filesOpt("icon", string);
    let readmeUrl = files("readme", nullable(string));
    let homepageUrl = field("publishedBy", field("homepage", string));

    let decode =
      obj(({field, whatever, _}) =>
        {
          downloadUrl: whatever(downloadUrl),
          manifestUrl: whatever(manifestUrl),
          iconUrl: whatever(iconUrl),
          readmeUrl: whatever(readmeUrl),
          repositoryUrl: field.optional("repository", string),
          homepageUrl: whatever(homepageUrl),
          licenseName: field.optional("license", string),
          displayName: field.optional("displayName", string),
          description: field.optional("description", string),
          name: field.required("name", string),
          namespace: field.required("namespace", string),
          version: field.required("version", string),
          versions: field.withDefault("allVersions", [], VersionInfo.decode),
        }
      );
  };
};

let details = (~setup, {publisher, name}: Identifier.t) => {
  let url = Url.extensionInfo(publisher, name);
  Service_Net.Request.json(~setup, ~decoder=Details.Decode.decode, url);
};

module Summary = {
  [@deriving show]
  type t = {
    url: string,
    downloadUrl: string,
    iconUrl: option(string),
    version: string,
    name: string,
    namespace: string,
    displayName: option(string),
    description: option(string),
  };

  let id = ({namespace, name, _}) => {
    namespace ++ "." ++ name;
  };

  let name = ({displayName, _} as summary) => {
    let default = summary |> id;
    displayName |> Option.value(~default);
  };

  let decode = {
    open Json.Decode;

    let downloadUrl = field("files", field("download", string));
    let iconUrl = field("files", field_opt("icon", string));

    obj(({field, whatever, _}) =>
      {
        url: field.required("url", string),
        downloadUrl: whatever(downloadUrl),
        iconUrl: whatever(iconUrl),
        version: field.required("version", string),
        name: field.required("name", string),
        namespace: field.required("namespace", string),
        displayName: field.optional("displayName", string),
        description: field.optional("description", string),
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
      displayName |> Option.value(~default="(null)"),
      description |> Option.value(~default="(null)"),
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
