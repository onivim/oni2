module Core = Oni_Core;

module Ripgrep = Core.Ripgrep;
module Subscription = Core.Subscription;
module Log = (val Core.Log.withNamespace("Oni2.Search.SearchSubscription"));

module Make = (Config: {type action;}) => {
  module Provider = {
    include Config;

    type params = {
      directory: string,
      query: string,
      ripgrep: Ripgrep.t, // TODO: Necessary dependency?
      onUpdate: list(Ripgrep.Match.t) => unit, // TODO: Should return action
      onCompleted: unit => action,
    };

    let jobs = Hashtbl.create(10);

    let start =
        (
          ~id,
          ~params as {directory, query, ripgrep, onUpdate, onCompleted},
          ~dispatch: _,
        ) => {
      Log.debug("Starting " ++ id);

      let dispose =
        ripgrep.Ripgrep.findInFiles(
          ~directory, ~query, ~onUpdate, ~onComplete=() => {
          dispatch(onCompleted())
        });

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

  let create = (~id, ~directory, ~query, ~ripgrep, ~onUpdate, ~onCompleted) =>
    Subscription.create(
      id,
      (module Provider),
      {directory, query, ripgrep, onUpdate, onCompleted},
    );
};
