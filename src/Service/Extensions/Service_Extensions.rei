open Oni_Core;

module Catalog: {
  module Identifier: {
    [@deriving show]
    type t = {
      publisher: string,
      name: string,
    };

    let fromString: string => option(t);
    let toString: t => string;
  };

  module VersionInfo: {
    [@deriving show]
    type t = {
      version: Semver.t,
      url: string,
    };
  };

  module Details: {
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
      version: option(Semver.t),
      versions: list(VersionInfo.t),
    };

    let toString: t => string;
  };

  module Summary: {
    [@deriving show]
    type t = {
      url: string,
      downloadUrl: string,
      iconUrl: option(string),
      version: option(Semver.t),
      name: string,
      namespace: string,
      displayName: option(string),
      description: option(string),
    };

    let name: t => string;

    let id: t => string;

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
    (~setup: Setup.t, ~extensionsFolder: Fp.t(Fp.absolute)=?, string) =>
    Lwt.t(unit);

  let uninstall:
    (~extensionsFolder: Fp.t(Fp.absolute)=?, string) => Lwt.t(unit);

  let get:
    (~extensionsFolder: Fp.t(Fp.absolute)=?, unit) =>
    Lwt.t(list(Exthost.Extension.Scanner.ScanResult.t));
};

module Query: {
  [@deriving show]
  type t;
  let create: (~searchText: string) => t;

  let isComplete: t => bool;
  let searchText: t => string;
  let percentComplete: t => float;
  let results: t => list(Catalog.Summary.t);
};

module Effects: {
  let uninstall:
    (
      ~extensionsFolder: option(Fp.t(Fp.absolute)),
      ~toMsg: result(unit, string) => 'a,
      string
    ) =>
    Isolinear.Effect.t('a);

  let install:
    (
      ~extensionsFolder: option(Fp.t(Fp.absolute)),
      ~toMsg: result(Exthost.Extension.Scanner.ScanResult.t, string) => 'a,
      string
    ) =>
    Isolinear.Effect.t('a);

  let update:
    (
      ~extensionsFolder: option(Fp.t(Fp.absolute)),
      ~toMsg: result(Exthost.Extension.Scanner.ScanResult.t, string) => 'msg,
      string
    ) =>
    Isolinear.Effect.t('msg);

  let details:
    (~extensionId: string, ~toMsg: result(Catalog.Details.t, string) => 'a) =>
    Isolinear.Effect.t('a);
};

module Sub: {
  let search:
    (~setup: Setup.t, ~query: Query.t, ~toMsg: result(Query.t, exn) => 'a) =>
    Isolinear.Sub.t('a);

  let details:
    (
      ~setup: Setup.t,
      ~extensionId: string,
      ~toMsg: result(Catalog.Details.t, string) => 'a
    ) =>
    Isolinear.Sub.t('a);
};
