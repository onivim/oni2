module Schema: {
  type menu;

  type item;
  type group;

  let item: (~title: string, ~command: string) => item;
  let submenu: (~title: string, list(group)) => item;

  let command: (~title: string=?, Command.t(_)) => item;

  let group: (~order: int=?, ~parent: menu, list(item)) => group;

  let menu:
    (~order: int=?, ~uniqueId: string, ~parent: option(menu), string) => menu;

  type t;
  let initial: t;

  let menus: list(menu) => t;
  let groups: list(group) => t;

  let union: (t, t) => t;
  let ofList: list(t) => t;
};

type builtMenu;

module Item: {
  type t = Schema.item;

  let title: t => string;
  let command: t => string;

  let isSubmenu: t => bool;
  let submenu: t => list(Schema.group);
};

module Group: {
  type t = Schema.group;

  let items: t => list(Item.t);
};

module Menu: {
  type t;

  let title: t => string;
  let uniqueId: t => string;

  let contents: (t, builtMenu) => list(Group.t);
};

let build:
  (
    ~contextKeys: WhenExpr.ContextKeys.t,
    ~commands: Command.Lookup.t(_),
    Schema.t
  ) =>
  builtMenu;

let schema: builtMenu => Schema.t;

// [top(builtMenu)] returns the top-level menu items for [builtMenu]
let top: Schema.t => list(Menu.t);
