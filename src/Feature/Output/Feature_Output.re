[@deriving show]
type msg =
  | Pane(Pane.msg);

type model = {pane: Pane.t};

let initial = {pane: Pane.initial};

type outmsg = Pane.outmsg = | Nothing | ClosePane;

let update = (msg, model) =>
  switch (msg) {
  | Pane(msg) =>
    let (pane', outmsg) = Pane.update(msg, model.pane);
    ({pane: pane'}, outmsg);
  };

let setProcessOutput = (~cmd, ~output, model) => {
  let pane' = Pane.set(~cmd, ~maybeContents=output, model.pane);

  {pane: pane'};
};

module Contributions = {
  let pane: Feature_Pane.Schema.t(model, msg) = {
    Pane.pane
    |> Feature_Pane.Schema.map(
         ~model=({pane, _}) => pane,
         ~msg=msg => Pane(msg),
       );
  };
};
