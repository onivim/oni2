module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Ripgrep = Core.Ripgrep;
module Subscription = Core.Subscription;
module Log = (val Core.Log.withNamespace("Oni2.Store.SearchSubscription"));

module Provider = {
  type action = Actions.t;
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
    Log.info("Starting Search subscription " ++ id);

    let dispose =
      ripgrep.Ripgrep.findInFiles(
        ~directory,
        ~query,
        ~onUpdate,
        ~onComplete=() => {
          Log.info("Ripgrep completed.");
          dispatch(onCompleted());
        },
      );

    Hashtbl.replace(jobs, id, (query, dispose));
  };

  let update = (~id, ~params, ~dispatch) => {
    switch (Hashtbl.find_opt(jobs, id)) {
    | Some((currentQuery, dispose)) =>
      if (currentQuery != params.query) {
        Log.info("Updating Search subscription " ++ id);
        dispose();
        start(~id, ~params, ~dispatch);
      }

    | None => Log.error("Tried to dispose non-existing Search subscription")
    };
  };

  let dispose = (~id) => {
    switch (Hashtbl.find_opt(jobs, id)) {
    | Some((_, dispose)) =>
      Log.info("Disposing Search subscription " ++ id);
      dispose();
      Hashtbl.remove(jobs, id);

    | None => Log.error("Tried to dispose non-existing Search subscription")
    };
  };
};

let create = (~id, ~directory, ~query, ~ripgrep, ~onUpdate, ~onCompleted) =>
  Subscription.create(
    id,
    (module Provider),
    {directory, query, ripgrep, onUpdate, onCompleted},
  );
