module Make = (JobConfig: Oni_Model.FilterJob.Config) => {
  module Core = Oni_Core;
  module Model = Oni_Model;
  module Log = Core.Log;

  module Actions = Model.Actions;
  module Subscription = Core.Subscription;
  module Job = Core.Job;

  module FilterJob = Model.FilterJob.Make(JobConfig);

  module Provider = {
    type action = Actions.t;

    type params = {
      query: string,
      items: list(JobConfig.item),
      itemStream: Isolinear.Stream.t(list(JobConfig.item)),
      onUpdate:
        (list(Model.Filter.result(JobConfig.item)), ~progress: float) =>
        action,
    };

    type state = {
      job: FilterJob.t,
      dispose: unit => unit,
    };

    let jobs = Hashtbl.create(10);

    let start =
        (~id, ~params as {query, items, itemStream, onUpdate}, ~dispatch) => {
      Log.debug(() => "Starting FilterJob subscription " ++ id);
      let job =
        FilterJob.create()
        |> Job.map(FilterJob.updateQuery(query))
        |> Job.map(FilterJob.addItems(items));

      let unsubscribeFromItemStream =
        Isolinear.Stream.subscribe(itemStream, items =>
          switch (Hashtbl.find_opt(jobs, id)) {
          | Some({job, _} as state) =>
            let job = Job.map(FilterJob.addItems(items), job);
            Hashtbl.replace(jobs, id, {...state, job});

          | None => Log.error("Unable to add items to non-existing FilterJob")
          }
        );

      let disposeTick =
        Revery.Tick.interval(
          _ =>
            switch (Hashtbl.find_opt(jobs, id)) {
            | Some({job, _} as state) =>
              if (!Job.isComplete(job)) {
                Hashtbl.replace(jobs, id, {...state, job: Job.tick(job)});
              }

            | None => Log.error("Unable to tick non-existing FilterJob")
            },
          Seconds(0.),
        );

      let disposeMessagePump =
        Revery.Tick.interval(
          _ =>
            switch (Hashtbl.find_opt(jobs, id)) {
            | Some({job, _}) =>
              let items = Job.getCompletedWork(job).uiFiltered;
              let progress = Job.getProgress(job);
              dispatch(onUpdate(items, ~progress));

            | None => Log.error("Unable to pump non-existing FilterJob")
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

    let update = (~id, ~params as {query, _}, ~dispatch as _) =>
      switch (Hashtbl.find_opt(jobs, id)) {
      | Some({job, _} as state) when query != job.pendingWork.filter =>
        // Query changed
        Log.debug(() =>
          "Updating FilterJob subscription " ++ id ++ " with query: " ++ query
        );
        let job = Job.map(FilterJob.updateQuery(query), job);
        Hashtbl.replace(jobs, id, {...state, job});

      | Some(_) => () // Query hasn't changed, so do nothing

      | None =>
        Log.error("Unable to update non-existing FilterJob subscription")
      };

    let dispose = (~id) => {
      switch (Hashtbl.find_opt(jobs, id)) {
      | Some({dispose, _}) =>
        Log.debug(() => "Disposing FilterJob subscription " ++ id);
        dispose();
        Hashtbl.remove(jobs, id);

      | None =>
        Log.error(
          "Tried to dispose non-existing FilterJob subscription: " ++ id,
        )
      };
    };
  };

  let create = (~id, ~query, ~items, ~itemStream, ~onUpdate) =>
    Subscription.create(
      id,
      (module Provider),
      {query, items, itemStream, onUpdate},
    );
};
