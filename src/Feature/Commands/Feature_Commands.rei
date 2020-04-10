open Oni_Core;

module Schema: {
  let define:
    (
      ~category: string=?,
      ~title: string=?,
      ~icon: IconTheme.IconDefinition.t=?,
      ~isEnabledWhen: WhenExpr.t=?,
      string,
      'msg
    ) =>
    Command.t('msg);
};

// MODEL

type model('msg);

let initial: list(Command.t('msg)) => model('msg);

let find: (string, model('msg)) => option(Command.t('msg));

let all: model('msg) => list(Command.t('msg));
let enabledCommands:
  (WhenExpr.ContextKeys.t, model('msg)) => list(Command.t('msg));

// UPDATE

[@deriving show]
type msg('msg) =
  | NewCommand(Command.t('msg));

let update: (model('msg), msg('msg)) => model('msg);
