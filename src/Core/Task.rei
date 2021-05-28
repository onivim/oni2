[@deriving show]
type msg;

type model;

let initial: unit => model;

type task = unit => unit;

let queueTask: (~task: task, model) => model;

let update: (msg, model) => model;

let sub: model => Isolinear.Sub.t(msg);
