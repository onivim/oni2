open Oni_Core;
open Utility;

module Catalog = Service_Extensions.Catalog;

open TestFramework;

describe("Catalog", ({describe, _}) => {
  let setup = Setup.init();
  let proxy = Service_Net.Proxy.none;

  let searchSync = searchString => {
    let rec loop = (acc, offset) => {
      let {offset, totalSize, extensions}: Catalog.SearchResponse.t =
        LwtEx.sync(Catalog.search(~proxy, ~offset, ~setup, searchString))
        |> Result.get_ok;

      let allExtensions = extensions @ acc;
      if (List.length(extensions) + offset >= totalSize) {
        allExtensions;
      } else {
        loop(allExtensions, offset + List.length(extensions));
      };
    };

    loop([], 0);
  };

  describe("search", ({test, _}) => {
    test("#2723 - extension search failing for 'theme'", ({expect, _}) => {
      let extensions = searchSync("theme");
      expect.equal(List.length(extensions) > 1, true);
    })
  });
});
