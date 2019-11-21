module Core = Oni_Core;
module Model = Oni_Model;
module Log = Core.Log;

module Actions = Model.Actions;
module Ripgrep = Core.Ripgrep;
module Subscription = Core.Subscription;

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
        directory,
        query,
        onUpdate,
        () => {
          Log.info("Ripgrep completed.");
          dispatch(onCompleted());
        },
      );

    Hashtbl.add(jobs, id, dispose);
  };

  let update = (~id as _, ~params as _, ~dispatch as _) => (); // Nothing to update

  let dispose = (~id) => {
    switch (Hashtbl.find_opt(jobs, id)) {
    | Some(dispose) =>
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
