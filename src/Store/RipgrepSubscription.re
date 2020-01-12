module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Ripgrep = Core.Ripgrep;
module Subscription = Core.Subscription;
module Log = (val Core.Log.withNamespace("Oni2.Store.RipgrepSubscription"));

module Provider = {
  type action = Actions.t;
  type params = {
    filesExclude: list(string),
    directory: string,
    ripgrep: Ripgrep.t, // TODO: Necessary dependency?
    onUpdate: list(string) => unit, // TODO: Should return action
    onComplete: unit => action,
  };

  let jobs = Hashtbl.create(10);

  let start =
      (
        ~id,
        ~params as {filesExclude, directory, ripgrep, onUpdate, onComplete},
        ~dispatch: _,
      ) => {
    Log.info("Starting Ripgrep search subscription " ++ id);

    let dispose =
      ripgrep.Ripgrep.search(
        ~filesExclude,
        ~directory,
        ~onUpdate,
        ~onComplete=() => {
          Log.info("Ripgrep completed.");
          dispatch(onComplete());
        },
      );

    Hashtbl.add(jobs, id, dispose);
  };

  let update = (~id as _, ~params as _, ~dispatch as _) => (); // Nothing to update

  let dispose = (~id) => {
    switch (Hashtbl.find_opt(jobs, id)) {
    | Some(dispose) =>
      Log.info("Disposing subscription " ++ id);
      dispose();
      Hashtbl.remove(jobs, id);

    | None => Log.error("Tried to dispose non-existing Ripgrep subscription")
    };
  };
};

let create =
    (~id, ~filesExclude, ~directory, ~ripgrep, ~onUpdate, ~onComplete) =>
  Subscription.create(
    id,
    (module Provider),
    {filesExclude, directory, ripgrep, onUpdate, onComplete},
  );
