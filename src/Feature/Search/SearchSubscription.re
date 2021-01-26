module Core = Oni_Core;

module Ripgrep = Core.Ripgrep;
module Subscription = Core.Subscription;
module Log = (val Core.Log.withNamespace("Oni2.Search.SearchSubscription"));

module Make = (Config: {type action;}) => {
  module Provider = {
    include Config;

    type params = {
      searchExclude: list(string),
      directory: string,
      query: string,
      ripgrep: Ripgrep.t, // TODO: Necessary dependency?
      onUpdate: list(Ripgrep.Match.t) => unit, // TODO: Should return action
      onCompleted: unit => action,
      onError: string => action,
    };

    let jobs = Hashtbl.create(10);

    let start =
        (
          ~id,
          ~params as {
            searchExclude,
            directory,
            query,
            ripgrep,
            onUpdate,
            onCompleted,
            onError,
          },
          ~dispatch: _,
        ) => {
      Log.info("Starting " ++ id);

      let dispose =
        ripgrep.Ripgrep.findInFiles(
          ~searchExclude,
          ~directory,
          ~query,
          ~onUpdate,
          ~onComplete=() => {dispatch(onCompleted())},
          ~onError=msg => dispatch(onError(msg)),
        );

      Hashtbl.replace(jobs, id, (query, dispose));
    };

    let update = (~id, ~params, ~dispatch) => {
      switch (Hashtbl.find_opt(jobs, id)) {
      | Some((currentQuery, dispose)) =>
        if (currentQuery != params.query) {
          Log.info("Updating " ++ id);
          dispose();
          start(~id, ~params, ~dispatch);
        }

      | None => Log.warn("Tried to dispose non-existing instance " ++ id)
      };
    };

    let dispose = (~id) => {
      switch (Hashtbl.find_opt(jobs, id)) {
      | Some((_, dispose)) =>
        Log.info("Disposing " ++ id);
        dispose();
        Hashtbl.remove(jobs, id);

      | None => Log.warn("Tried to dispose non-existing instance " ++ id)
      };
    };
  };

  let create =
      (
        ~id,
        ~searchExclude,
        ~directory,
        ~query,
        ~ripgrep,
        ~onUpdate,
        ~onCompleted,
        ~onError,
      ) =>
    Subscription.create(
      id,
      (module Provider),
      {
        searchExclude,
        directory,
        query,
        ripgrep,
        onUpdate,
        onCompleted,
        onError,
      },
    );
};
