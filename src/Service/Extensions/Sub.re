open Oni_Core;
type msg = result(Query.t, string);
type params = {
  setup: Setup.t,
  query: Query.t,
};
module SearchSub =
  Isolinear.Sub.Make({
    type nonrec msg = msg;

    type nonrec params = params;

    type state = unit;

    let name = "Service_Extensions.SearchSub";
    let id = ({query, _}) => {
      query.searchText ++ "." ++ string_of_int(query.offset);
    };

    let init = (~params, ~dispatch) => {
      let {query, setup} = params;
      let result =
        Catalog.search(~offset=query.offset, ~setup, query.searchText);

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
              items: extensions @ query.items,
            };
          dispatch(Ok(newQuery));
        },
      );

      Lwt.on_failure(result, err => {
        dispatch(Error(Printexc.to_string(err)))
      });
    };

    let update = (~params as _, ~state, ~dispatch as _) => {
      state;
    };

    let dispose = (~params as _, ~state as _) => {
      ();
    };
  });

let search = (~setup, ~query, ~toMsg) => {
  SearchSub.create({query, setup}) |> Isolinear.Sub.map(toMsg);
};
