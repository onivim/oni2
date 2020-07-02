open Oni_Core;
module Catalog: {
  module VersionInfo: {
    type t = {
      version: string,
      url: string,
    };
  };

  module Entry: {
    type t = {
      downloadUrl: string,
      repositoryUrl: string,
      homepageUrl: string,
      manifestUrl: string,
      iconUrl: string,
      readmeUrl: string,
      licenseName: string,
      //      licenseUrl: string,
      //      name: string,
      //      namespace: string,
      //      downloadCount: int,
      displayName: string,
      description: string,
      //      categories: list(string),
      versions: list(VersionInfo.t),
    };

    let toString: t => string;
  };

  let query: (~setup: Setup.t, string) => Lwt.t(Entry.t);
};
