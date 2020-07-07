open Oni_Core;

module Catalog: {
  module Identifier: {
    type t = {
      publisher: string,
      name: string,
    };

    let fromString: string => option(t);
    let toString: t => string;
  };

  module VersionInfo: {
    type t = {
      version: string,
      url: string,
    };
  };

  module Details: {
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

    let toString: t => string;
  };

  module Summary: {
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

    let toString: t => string;
  };

  module SearchResponse: {
    type t = {
      offset: int,
      totalSize: int,
      extensions: list(Summary.t),
    };

    let toString: t => string;
  };

  let details: (~setup: Setup.t, Identifier.t) => Lwt.t(Details.t);
  let search:
    (~offset: int, ~setup: Setup.t, string) => Lwt.t(SearchResponse.t);
};

module Management: {
  let install:
    (~setup: Setup.t, ~extensionsFolder: string=?, string) => Lwt.t(string);

  let uninstall: (~extensionsFolder: string=?, string) => Lwt.t(unit);

  let get:
    (~extensionsFolder: string=?, unit) =>
    Lwt.t(list(Exthost.Extension.Scanner.ScanResult.t));
};

module Query: {
  type t;
  let create: (~searchText: string) => t;

  let isComplete: t => bool;
  let percentComplete: t => float;
  let results: t => list(Catalog.Summary.t);
};

module Sub: {
  let search: (
    ~setup: Setup.t,
    ~query: Query.t,
    ~toMsg: result(Query.t, string) => 'a)
    => Isolinear.Sub.t('a)
}
