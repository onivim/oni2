module Core = Oni_Core;
module Model = Oni_Model;
module Log = Core.Log;

module Actions = Model.Actions;
module Subscription = Core.Subscription;
module Job = Core.Job;

module MenuJob = Model.FilterJob.Make({
  type item = Actions.menuCommand;
  let rank = Model.Filter.rank;
  let format = Model.Quickmenu.getLabel;
});

module Provider = {
  type action = Actions.t;

  type params = {
    query: string,
    items: list(Actions.menuCommand),
    itemStream: Isolinear.Stream.t(list(Actions.menuCommand)),
    onUpdate: (list(Actions.menuCommand), ~progress: float) => action
  };

  type state = {
    job: MenuJob.t,
    dispose: unit => unit
  }

  let jobs = Hashtbl.create(10);

  let start = (~id, ~params as {query, items, itemStream, onUpdate}, ~dispatch) => {
    Log.debug("Starting MenuJob subscription " ++ id);
    let job =
      MenuJob.create()
        |> Job.map(MenuJob.updateQuery(query))
        |> Job.map(MenuJob.addItems(items))
        |> Job.doWork;

    let unsubscribeFromItemStream =
      Isolinear.Stream.subscribe(itemStream, items =>
        switch (Hashtbl.find_opt(jobs, id)) {
          | Some({ job } as state) =>
            let job = Job.map(MenuJob.addItems(items), job) |> Job.doWork;
            Hashtbl.replace(jobs, id, { ...state, job });

          | None =>
            Log.error("Unable to add items to non-existing MenuJob");
        },
      );

    let disposeTick = 
      Revery.Tick.interval(
        _ =>
          switch (Hashtbl.find_opt(jobs, id)) {
            | Some({ job } as state) =>
              if (!Job.isComplete(job)) {
                Hashtbl.replace(jobs, id, { ...state, job: Job.tick(job) });
              }

            | None =>
              Log.error("Unable to tick non-existing MenuJob");
          },
        Seconds(0.),
      );

    let disposeMessagePump = 
      Revery.Tick.interval(
        _ =>
          switch (Hashtbl.find_opt(jobs, id)) {
            | Some({ job }) =>
              let items =
                Job.getCompletedWork(job).uiFiltered
                  |> List.map((Model.FilterJob.{ item, highlight}) => Actions.{ ...item, highlight })
              let progress =
                Job.getProgress(job);
              dispatch(onUpdate(items, progress));

            | None =>
              Log.error("Unable to pump non-existing MenuJob");
          },
        Seconds(0.1),
      );

    let dispose = () => {
      unsubscribeFromItemStream();
      disposeTick();
      disposeMessagePump();
    };

    Hashtbl.add(jobs, id, {job, dispose});
  };

  let update = (~id, ~params as {query}, ~dispatch) =>
    switch (Hashtbl.find_opt(jobs, id)) {
      | Some({ job } as state) =>
        // `MenuJob.updateQuery` checks if `query` has changed, so we don't need to. It would make the commented out log message below less annoying though
        /* Log.debug("Updating MenuJob subscription " ++ id); */
        let job = Job.map(MenuJob.updateQuery(query), job) |> Job.doWork;
        Hashtbl.replace(jobs, id, { ...state, job });

      | None =>
        Log.error("Unable to update non-existing MenuJob subscription");
    };

  let dispose = (~id) => {
    switch (Hashtbl.find_opt(jobs, id)) {
      | Some({ job, dispose }) =>
        Log.debug("Disposing MenuJob subscription " ++ id);
        dispose();
        Hashtbl.remove(jobs, id);

      | None =>
        Log.error("Tried to dispose non-existing MenuJob subscription: " ++ id);
    };
  };
};

let create = (~id, ~query, ~items, ~itemStream, ~onUpdate) =>
  Subscription.create(id, (module Provider), { query, items, itemStream, onUpdate });