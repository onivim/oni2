open Oni_Core;

module Schema = {
  [@deriving show]
  type command('msg) = {
    id: string,
    title: option(string),
    category: option(string),
    icon: option([@opaque] IconTheme.IconDefinition.t),
    isEnabledWhen: WhenExpr.t,
    msg: [ | `Arg0('msg) | `Arg1(Json.t => 'msg)],
  };

  let map = (f, command) => {
    ...command,
    msg:
      switch (command.msg) {
      | `Arg0(msg) => `Arg0(f(msg))
      | `Arg1(msgf) => `Arg1(arg => f(msgf(arg)))
      },
  };

  let define =
      (
        ~category=?,
        ~title=?,
        ~icon=?,
        ~isEnabledWhen=WhenExpr.Value(True),
        id,
        msg,
      ) => {
    id,
    title,
    category,
    icon,
    isEnabledWhen,
    msg: `Arg0(msg),
  };
};

// MODEL

type model('msg) = StringMap.t(Schema.command('msg));

let initial = contributions =>
  contributions
  |> List.to_seq
  |> Seq.map(Schema.(command => (command.id, command)))
  |> StringMap.of_seq;

let find = StringMap.find_opt;

let enabledCommands = (getValue, model) =>
  model
  |> StringMap.to_seq
  |> Seq.map(snd)
  |> Seq.filter(
       Schema.(it => WhenExpr.evaluate(it.isEnabledWhen, getValue)),
     )
  |> List.of_seq;

// UPDATE

[@deriving show]
type msg('msg) =
  | NewCommand(Schema.command('msg));

let update = (model, msg) => {
  switch (msg) {
  | NewCommand(command) => StringMap.add(command.id, command, model) /*   }*/
  };
};
