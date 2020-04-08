open Oni_Core;

module Schema = {
  [@deriving show]
  type command('msg) = {
    id: string,
    title: option(string),
    category: option(string),
    icon: option([@opaque] IconTheme.IconDefinition.t),
    isEnabled: unit => bool, // WhenExpr.t
    msg: 'msg,
  };

  let map = (f, command) => {...command, msg: f(command.msg)};

  let define =
      (~category=?, ~title=?, ~icon=?, ~isEnabled=() => true, id, msg) => {
    id,
    title,
    category,
    icon,
    isEnabled,
    msg,
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

let enabledCommands = model =>
  model
  |> StringMap.to_seq
  |> Seq.map(snd)
  |> Seq.filter(Schema.(it => it.isEnabled()))
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
