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

let initial: list(Command.Lookup.t('msg)) => model('msg);

let all: model('msg) => Command.Lookup.t('msg);

// UPDATE

[@deriving show]
type msg('msg) =
  | NewCommand(Command.t('msg));

let update: (model('msg), msg('msg)) => model('msg);
