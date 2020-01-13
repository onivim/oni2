/*
 * LanguageFeature.re
 */

open Oni_Core;

module Make =
       (
         Provider: {
           type params;
           type response;

           let namespace: string;

           let aggregate: list(Lwt.t(response)) => Lwt.t(response);
         },
       ) => {
  module Log = (
    val Log.withNamespace("Oni2.Model.LanguageFeature:" ++ Provider.namespace)
  );

  module Params = {
    type t = Provider.params;
  };
  type response = Provider.response;

  type t = Params.t => option(Lwt.t(response));

  type info = {
    provider: t,
    id: string,
  };

  type providers = list(info);

  let register = (~id: string, provider: t, providers: providers) => {
    [{id, provider}, ...providers];
  };

  let get = (providers: providers) => {
    providers |> List.map(({id, _}) => id);
  };

  let request = (params: Params.t, providers: providers) => {
    let promises =
      providers
      |> List.map(({id, provider}) => {
           let result = provider(params);
           switch (result) {
           | Some(_) => Log.infof(m => m("Querying provider: %s", id))
           | None => Log.debugf(m => m("Provider skipped: %s", id))
           };
           result;
         })
      |> Utility.Option.values;

    Provider.aggregate(promises);
  };
};
