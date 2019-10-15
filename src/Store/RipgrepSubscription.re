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
    ripgrep: Ripgrep.t, // TODO: Necessary dependency?
    onUpdate: list(string) => unit, // TODO: Should return action
    onCompleted: unit => action
  };

  let jobs = Hashtbl.create(10);

  let start = (~id, ~params as {directory, ripgrep, onUpdate, onCompleted}, ~dispatch:_) => {
    Log.info("Starting Ripgrep search subscription " ++ id);

    let dispose =
      ripgrep.Ripgrep.search(
        directory,
        onUpdate,
        () => {
          Log.info("[QuickOpenStoreConnector] Ripgrep completed.");
          dispatch(onCompleted());
        },
      );

    Hashtbl.add(jobs, id, dispose);
  };

  let update = (~id, ~params as _, ~dispatch) =>
    (); // Nothing to update

  let dispose = (~id) => {
    switch (Hashtbl.find_opt(jobs, id)) {
      | Some(dispose) =>
        Log.info("Disposing Ripgrep subscription " ++ id);
        dispose();
        Hashtbl.remove(jobs, id);

      | None =>
        Log.error("Tried to dispose non-existing Ripgrep subscription");
    };
  };
};

let create = (~id, ~directory, ~ripgrep, ~onUpdate, ~onCompleted) =>
  Subscription.create(id, (module Provider), { directory, ripgrep, onUpdate, onCompleted });