module Schema: {
  type item;
  type menu;

  let item: (~title: string, ~command: string, ~parent: menu) => item;

  let menu:
    (~uniqueId: string, ~title: string, ~parent: option(menu)) => menu;

  type t;

  let toSchema: (~menus: list(menu)=?, ~items: list(item)) => t;
  let union: (t, t) => t;
};

type t;

let initial: Schema.t => t;

type builtMenu('msg);

let build:
  (
    ~contextKeys: WhenExpr.ContextKeys.t,
    ~commands: Command.Lookup.t('msg),
    t
  ) =>
  builtMenu('msg);
