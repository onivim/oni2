open Oni_Core;

module Schema: {
  [@deriving show]
  type command('msg) = {
    id: string,
    title: option(string),
    category: option(string),
    icon: option([@opaque] IconTheme.IconDefinition.t),
    isEnabled: WhenExpr.t,
    msg: [ | `Arg0('msg) | `Arg1(Json.t => 'msg)],
  };

  let map: ('a => 'b, command('a)) => command('b);

  let define:
    (
      ~category: string=?,
      ~title: string=?,
      ~icon: IconTheme.IconDefinition.t=?,
      ~isEnabled: WhenExpr.t=?,
      string,
      'msg
    ) =>
    command('msg);
};

// MODEL

type model('msg);

let initial: list(Schema.command('msg)) => model('msg);

let find: (string, model('msg)) => option(Schema.command('msg));

let enabledCommands:
  (string => WhenExpr.Value.t, model('msg)) => list(Schema.command('msg));

// UPDATE

[@deriving show]
type msg('msg) =
  | NewCommand(Schema.command('msg));

let update: (model('msg), msg('msg)) => model('msg);
