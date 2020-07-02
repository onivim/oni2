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
//      name: string,
//      namespace: string,
//      downloadCount: int,
//      displayName: string,
//      descrption: string,
//      categories: list(string),
//      license: string,
//      repositoryUrl: string,
//      versions: list(VersionInfo.t),
    }

    let toString: t => string;
  };

  let query: (
    ~setup: Setup.t,
    ~publisher: string,
    string) => Lwt.t(Entry.t);

};
