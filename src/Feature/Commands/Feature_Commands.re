open Oni_Core;

module Schema = {
  let define =
      (
        ~category=?,
        ~title=?,
        ~icon=?,
        ~isEnabledWhen=WhenExpr.Value(True),
        id,
        msg,
      ) =>
    Command.{id, title, category, icon, isEnabledWhen, msg: `Arg0(msg)};
};

// MODEL

type model('msg) = StringMap.t(Command.t('msg));

let initial = contributions =>
  contributions
  |> List.to_seq
  |> Seq.map(Command.(command => (command.id, command)))
  |> StringMap.of_seq;

let find = StringMap.find_opt;

let all = model => model |> StringMap.to_seq |> Seq.map(snd) |> List.of_seq;

let enabledCommands = (contextKeys, model) =>
  model
  |> StringMap.to_seq
  |> Seq.map(snd)
  |> Seq.filter(
       Command.(
         it =>
           WhenExpr.evaluate(
             it.isEnabledWhen,
             WhenExpr.ContextKeys.getValue(contextKeys),
           )
       ),
     )
  |> List.of_seq;

// UPDATE

[@deriving show]
type msg('msg) =
  | NewCommand(Command.t('msg));

let update = (model, msg) => {
  switch (msg) {
  | NewCommand(command) => StringMap.add(command.id, command, model) /*   }*/
  };
};
