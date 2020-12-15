module Schema: {

  type menu;
  type item;

  let item: (~title: string, ~command: string, ~parent: menu) => item;

  let menu:
    (~uniqueId: string, ~title: string, ~parent: option(menu)) => menu;

  type t;
  let initial: t;

  let menus: list(menu) => t;
  let items: list(item) => t;

  //let toSchema: (~menus: list(menu)=?, ~items: list(item)) => t;
  let union: (t, t) => t;
  let ofList: list(t) => t;
};

type builtMenu;

module Item: {
  type t;

  let title: t => string;
  let command: t => string;
}

module Menu: {
  type t;

  type contentItem = 
  | SubMenu(t)
  | Item(Item.t)

  let title: t => string;

  let contents: (t, builtMenu) => list(contentItem);
}

let build:
  (
    ~contextKeys: WhenExpr.ContextKeys.t,
    ~commands: Command.Lookup.t(_),
    Schema.t
  ) =>
  builtMenu;

// [top(builtMenu)] returns the top-level menu items for [builtMenu]
let top: builtMenu => list(Menu.t);
