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
    version: [@opaque] Semver.t,
    url: string,
  };

  let decode =
    Json.Decode.(
      key_value_pairs(string)
      |> map(
           List.filter_map(((maybeVersion, url)) => {
             maybeVersion
             |> Semver.of_string
             |> Option.map(version => {version, url})
           }),
         )
    );

  let toString = ({version, url}) =>
    Printf.sprintf(" - Version %s: %s", Semver.to_string(version), url);
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
    isPublicNamespace: bool,
    //      downloadCount: int,
    displayName: option(string),
    description: option(string),
    //      categories: list(string),
    version: [@opaque] option(Semver.t),
    versions: list(VersionInfo.t),
    downloadCount: option(int),
    averageRating: option(float),
    reviewCount: option(int),
  };

  let id = ({namespace, name, _}) => {
    namespace ++ "." ++ name;
  };

  let displayName = ({displayName, _} as details) => {
    displayName |> Option.value(~default=id(details));
  };

  let averageRating = ({averageRating, _}) =>
    averageRating |> Option.value(~default=0.);

  let downloadCount = ({downloadCount, _}) =>
    downloadCount |> Option.value(~default=0);

  let reviewCount = ({reviewCount, _}) =>
    reviewCount |> Option.value(~default=0);

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
          downloadCount: field.optional("downloadCount", int),
          averageRating: field.optional("averageRating", float),
          reviewCount: field.optional("reviewCount", int),
          isPublicNamespace:
            field.withDefault(
              "namespaceAccess",
              true,
              string |> map(str => {String.lowercase_ascii(str) == "public"}),
            ),
          name: field.required("name", string),
          namespace: field.required("namespace", string),
          version:
            field.withDefault(
              "version",
              None,
              string |> map(Semver.of_string),
            ),
          versions: field.withDefault("allVersions", [], VersionInfo.decode),
        }
      );
  };
};

let details = (~proxy, ~setup, {publisher, name}: Identifier.t) => {
  let url = Url.extensionInfo(publisher, name);
  Service_Net.Request.json(
    ~proxy,
    ~setup,
    ~decoder=Details.Decode.decode,
    url,
  );
};

module Summary = {
  [@deriving show]
  type t = {
    url: string,
    downloadUrl: string,
    iconUrl: option(string),
    version: [@opaque] option(Semver.t),
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
        version:
          field.withDefault(
            "version",
            None,
            string |> map(Semver.of_string),
          ),
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
      version
      |> Option.map(Semver.to_string)
      |> Option.value(~default="(null)"),
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

let search = (~proxy, ~offset, ~setup, query) => {
  let url = Url.search(~query, ~offset);
  Service_Net.Request.json(
    ~proxy,
    ~setup,
    ~decoder=SearchResponse.decode,
    url,
  );
};
