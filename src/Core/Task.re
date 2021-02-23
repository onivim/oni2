module UniqueId =
  Revery.UniqueId.Make({});

type instanceId = string;

[@deriving show]
type taskId = int;

type task = unit => unit;

[@deriving show]
type msg =
  | TaskCompleted(taskId);

type model = {
  instanceId,
  nextTaskId: int,
  tasks: list((taskId, task)),
};

let initial = () => {
  instanceId: UniqueId.getUniqueId() |> string_of_int,
  nextTaskId: 0,
  tasks: [],
};

let queueTask = (~task, model) => {
  ...model,
  nextTaskId: model.nextTaskId + 1,
  tasks: [(model.nextTaskId, task), ...model.tasks],
};

let update = (msg, model) =>
  switch (msg) {
  | TaskCompleted(completedTaskId) => {
      ...model,
      tasks:
        model.tasks
        |> List.filter(((taskId, _)) => taskId != completedTaskId),
    }
  };

module TaskSub = {
  type params = {
    instanceId,
    taskId,
    task,
  };

  module Sub =
    Isolinear.Sub.Make({
      type nonrec msg = msg;
      type nonrec params = params;
      type state = unit;

      let name = "Task.subscription";

      let id = ({instanceId, taskId, _}) => {
        Printf.sprintf("%s.%d", instanceId, taskId);
      };

      let init = (~params, ~dispatch) => {
        params.task();
        dispatch(TaskCompleted(params.taskId));
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params as _, ~state as _) => ();
    });

  let create = (~instanceId, ~taskId, ~task) =>
    Sub.create({instanceId, taskId, task});
};

let sub = ({instanceId, tasks, _}) => {
  let revTasks = tasks |> List.rev;

  List.nth_opt(revTasks, 0)
  |> Option.map(((taskId, task)) =>
       TaskSub.create(~taskId, ~task, ~instanceId)
     )
  |> Option.value(~default=Isolinear.Sub.none);
};
