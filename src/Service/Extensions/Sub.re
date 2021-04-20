open Oni_Core;
type msg = result(Query.t, exn);
type searchParams = {
  proxy: Service_Net.Proxy.t,
  setup: Setup.t,
  query: Query.t,
};
module SearchSub =
  Isolinear.Sub.Make({
    type nonrec msg = msg;

    type nonrec params = searchParams;

    type state = unit;

    let name = "Service_Extensions.SearchSub";
    let id = ({query, _}) => {
      query.searchText ++ "." ++ string_of_int(query.offset);
    };

    let init = (~params, ~dispatch) => {
      let {proxy, query, setup} = params;
      let result =
        Catalog.search(
          ~proxy,
          ~offset=query.offset,
          ~setup,
          query.searchText,
        );

      Lwt.on_success(
        result,
        ({offset, totalSize, extensions}: Catalog.SearchResponse.t) => {
          let newExtensionCount = List.length(extensions);
          let newQuery =
            Query.{
              offset: offset + newExtensionCount,
              maybeRemainingCount:
                Some(totalSize - offset - newExtensionCount),
              searchText: query.searchText,
              items: query.items @ extensions,
            };
          dispatch(Ok(newQuery));
        },
      );

      Lwt.on_failure(result, exn => {dispatch(Error(exn))});
    };

    let update = (~params as _, ~state, ~dispatch as _) => {
      state;
    };

    let dispose = (~params as _, ~state as _) => {
      ();
    };
  });

let search = (~proxy, ~setup, ~query, ~toMsg) => {
  SearchSub.create({proxy, query, setup}) |> Isolinear.Sub.map(toMsg);
};

// DETAILS
type detailParams = {
  proxy: Service_Net.Proxy.t,
  setup: Setup.t,
  extensionId: string,
};

type detailMsg = result(Catalog.Details.t, string);

module DetailsSub =
  Isolinear.Sub.Make({
    type nonrec msg = detailMsg;

    type nonrec params = detailParams;

    type state = unit;

    let name = "Service_Extensions.DetailsSub";
    let id = ({extensionId, _}) => {
      extensionId;
    };

    let init = (~params, ~dispatch) => {
      let {extensionId, setup, proxy} = params;
      let maybeIdentifier = Catalog.Identifier.fromString(extensionId);
      switch (maybeIdentifier) {
      | None => dispatch(Error("Invalid id: " ++ extensionId))
      | Some(identifier) =>
        let result = Catalog.details(~proxy, ~setup, identifier);

        Lwt.on_success(result, (details: Catalog.Details.t) => {
          dispatch(Ok(details))
        });

        Lwt.on_failure(result, exn => {
          dispatch(Error(Printexc.to_string(exn)))
        });
      };
      ();
    };

    let update = (~params as _, ~state, ~dispatch as _) => {
      state;
    };

    let dispose = (~params as _, ~state as _) => {
      ();
    };
  });

let details =
    (
      ~proxy,
      ~setup,
      ~extensionId,
      ~toMsg: result(Catalog.Details.t, string) => 'a,
    ) => {
  DetailsSub.create({proxy, extensionId, setup}) |> Isolinear.Sub.map(toMsg);
};
