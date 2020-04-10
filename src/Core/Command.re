[@deriving show]
type t('msg) = {
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
