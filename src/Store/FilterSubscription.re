module Make = (JobConfig: Oni_Model.FilterJob.Config) => {
  module Core = Oni_Core;
  module Model = Oni_Model;

  module Actions = Model.Actions;
  module Subscription = Core.Subscription;
  module Job = Core.Job;
  module Time = Revery_Core.Time;
  module Log = (val Oni_Core_Kernel.Log.withNamespace("Oni2.Store.FilterSubscription"));

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
      Log.debug("Starting: " ++ id);

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

          | None => Log.warn("Unable to add items to non-existing FilterJob")
          }
        );

      let disposeTick =
        Revery.Tick.interval(
          _ =>
            switch (Hashtbl.find_opt(jobs, id)) {
            | Some({job, _} as state) when !Job.isComplete(job) =>
              let job = Job.tick(job);
              let items = Job.getCompletedWork(job).ranked;
              let progress = Job.getProgress(job);
              dispatch(onUpdate(items, ~progress));
              Hashtbl.replace(jobs, id, {...state, job});

            | Some(_) => ()

            | None => Log.warn("Unable to tick non-existing FilterJob")
            },
          Time.zero,
        );

      let dispose = () => {
        unsubscribeFromItemStream();
        disposeTick();
      };

      Hashtbl.add(jobs, id, {job, dispose});
    };

    let update = (~id, ~params as {query, _}, ~dispatch as _) =>
      switch (Hashtbl.find_opt(jobs, id)) {
      | Some({job, _} as state) when query != job.pendingWork.filter =>
        // Query changed
        Log.debug("Updating " ++ id ++ " with query: " ++ query);

        let job = Job.map(FilterJob.updateQuery(query), job);
        Hashtbl.replace(jobs, id, {...state, job});

      | Some(_) => () // Query hasn't changed, so do nothing

      | None => Log.warn("Unable to update non-existing instance " ++ id)
      };

    let dispose = (~id) => {
      switch (Hashtbl.find_opt(jobs, id)) {
      | Some({dispose, _}) =>
        Log.debug("Disposing: " ++ id);
        dispose();
        Hashtbl.remove(jobs, id);

      | None => Log.warn("Tried to dispose non-existing instance: " ++ id)
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
