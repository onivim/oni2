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
    followSymlinks: bool,
    useIgnoreFiles: bool,
    directory: string,
    ripgrep: Ripgrep.t, // TODO: Necessary dependency?
    onUpdate: list(string) => unit, // TODO: Should return action
    onComplete: unit => action,
    onError: string => action,
  };

  let jobs = Hashtbl.create(10);

  let start =
      (
        ~id,
        ~params as {
          followSymlinks,
          useIgnoreFiles,
          filesExclude,
          directory,
          ripgrep,
          onUpdate,
          onComplete,
          onError,
        },
        ~dispatch: _,
      ) => {
    Log.debug("Starting: " ++ id);

    let dispose: unit => unit =
      ripgrep.Ripgrep.search(
        ~followSymlinks,
        ~useIgnoreFiles,
        ~filesExclude,
        ~directory,
        ~onUpdate,
        ~onComplete=
          () => {
            Log.debug("Ripgrep completed.");
            dispatch(onComplete());
          },
        ~onError=msg => dispatch(onError(msg)),
      );

    Hashtbl.add(jobs, id, dispose);
  };

  let update = (~id as _, ~params as _, ~dispatch as _) => (); // Nothing to update

  let dispose = (~id) => {
    switch (Hashtbl.find_opt(jobs, id)) {
    | Some(dispose) =>
      Log.debug("Disposing: " ++ id);
      dispose();
      Hashtbl.remove(jobs, id);

    | None => Log.warn("Tried to dispose non-existing instance " ++ id)
    };
  };
};

let create =
    (
      ~id,
      ~followSymlinks,
      ~useIgnoreFiles,
      ~filesExclude,
      ~directory,
      ~ripgrep,
      ~onUpdate,
      ~onComplete,
      ~onError,
    ) =>
  Subscription.create(
    id,
    (module Provider),
    {
      followSymlinks,
      useIgnoreFiles,
      filesExclude,
      directory,
      ripgrep,
      onUpdate,
      onComplete,
      onError,
    },
  );
