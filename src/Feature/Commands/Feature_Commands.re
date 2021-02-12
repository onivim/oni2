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

  let defineWithArgs =
      (
        ~category=?,
        ~title=?,
        ~icon=?,
        ~isEnabledWhen=WhenExpr.Value(True),
        id,
        toMsg,
      ) => {
    Command.{id, title, category, icon, isEnabledWhen, msg: `Arg1(toMsg)};
  };
};

// MODEL

type model('msg) = Command.Lookup.t('msg);

let initial = contributions => Command.Lookup.unionMany(contributions);

let all = model => model;

// UPDATE

[@deriving show]
type msg('msg) =
  | NewCommand(Command.t('msg));

let update = (model, msg) => {
  switch (msg) {
  | NewCommand(command) => Command.Lookup.add(command.id, command, model) /*   }*/
  };
};
