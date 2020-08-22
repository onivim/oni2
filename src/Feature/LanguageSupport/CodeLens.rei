type codeLens;

type model;

let initial: model;

[@deriving show]
type msg;

let register:
  (~handle: int, ~selector: Exthost.DocumentSelector.t, model) => model;

let unregister: (~handle: int, model) => model;

let update: (msg, model) => model;

let get: (~bufferId: int, model) => list(codeLens);

let lineNumber: codeLens => int;
let text: codeLens => string;

let sub:
  (~visibleBuffers: list(Oni_Core.Buffer.t), ~client: Exthost.Client.t, model) =>
  Isolinear.Sub.t(msg);
