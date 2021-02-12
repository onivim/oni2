open Kernel;

module Schema = {
  type uniqueId = list(string);

  type item =
    | Item({
        title: string,
        command: string,
      })
    | Submenu({
        title: string,
        items: list(group),
      })
  and group = {
    order: int,
    parentId: uniqueId,
    items: list(item),
  };
  // | Item(item)
  // | Group({ parentId: uniqueId, items: list(item) });

  type menu = {
    order: int,
    uniqueId,
    title: string,
  };

  let group = (~order=500, ~parent, items) => {
    {order, parentId: parent.uniqueId, items};
  };

  let idToString = uniqueId => uniqueId |> String.concat(".");

  let item = (~title, ~command) => Item({title, command});

  let submenu = (~title, items) => Submenu({title, items});

  let command = (~title=?, command: Command.t(_)) => {
    let title =
      switch (title) {
      | Some(v) => v
      | None => command.title |> Option.value(~default="(null)")
      };
    Item({command: command.id, title});
  };

  let menu = (~order=500, ~uniqueId, ~parent: option(menu), title) => {
    let uniqueId =
      switch (parent) {
      | None => [uniqueId]
      | Some(parent) => [uniqueId, ...parent.uniqueId]
      };

    {order, uniqueId, title};
  };

  type t = {
    menus: StringMap.t(menu),
    items: StringMap.t(list(group)),
  };

  let initial = {menus: StringMap.empty, items: StringMap.empty};

  let menus = menus => {
    let menuMap =
      menus
      |> List.fold_left(
           (acc, curr: menu) => {
             acc |> StringMap.add(idToString(curr.uniqueId), curr)
           },
           StringMap.empty,
         );

    {menus: menuMap, items: StringMap.empty};
  };

  let groups = groups => {
    let itemsMap =
      groups
      |> List.fold_left(
           (acc, curr: group) => {
             acc
             |> StringMap.update(
                  idToString(curr.parentId),
                  fun
                  | None => Some([curr])
                  | Some(items) => Some([curr, ...items]),
                )
           },
           StringMap.empty,
         );

    {menus: StringMap.empty, items: itemsMap};
  };

  let union = (map1, map2) => {
    let mergeGreedy = (_key, maybeA, maybeB) =>
      switch (maybeA, maybeB) {
      | (Some(_) as a, _) => a
      | (None, Some(_) as b) => b
      | (None, None) => None
      };

    let mergeLists = (_key, maybeA, maybeB) =>
      switch (maybeA, maybeB) {
      | (Some(aList), Some(bList)) => Some(aList @ bList)
      | (None, Some(list)) => Some(list)
      | (Some(list), None) => Some(list)
      | (None, None) => None
      };
    let items' = StringMap.merge(mergeLists, map1.items, map2.items);

    let menus' = StringMap.merge(mergeGreedy, map1.menus, map2.menus);

    {menus: menus', items: items'};
  };

  let ofList = schemas => {
    schemas |> List.fold_left((acc, curr) => {union(acc, curr)}, initial);
  };
};

type t = Schema.t;

module Item = {
  open Schema;
  type t = Schema.item;

  let title =
    fun
    | Item({title, _}) => title
    | Submenu({title, _}) => title;

  let command =
    fun
    | Item({command, _}) => command
    | Submenu(_) => "___submenu___";

  let isSubmenu =
    fun
    | Item(_) => false
    | Submenu(_) => true;

  let submenu =
    fun
    | Item(_) => []
    | Submenu({items, _}) => items;
};

module Group = {
  type t = Schema.group;

  let items = ({items, _}: Schema.group) => items;
};

type builtMenu = {schema: t};

module Menu = {
  type t = Schema.menu;

  let title = ({title, _}: Schema.menu) => title;
  let uniqueId = ({uniqueId, _}: Schema.menu) =>
    uniqueId |> Schema.idToString;

  let contents = (menu: t, builtMenu) => {
    StringMap.find_opt(uniqueId(menu), builtMenu.schema.items)
    |> Option.value(~default=[])
    |> List.sort((a: Schema.group, b) => Schema.(a.order - b.order));
  };
};

let build = (~contextKeys as _, ~commands as _, menu) => {schema: menu};

let schema = ({schema, _}) => schema;

let top = schema => {
  Schema.(
    {
      schema.menus
      |> StringMap.bindings
      |> List.map(snd)
      |> List.sort((a, b) => Schema.(a.order - b.order));
    }
  );
};
