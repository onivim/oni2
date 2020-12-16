open Kernel;

module Schema = {
  type uniqueId = list(string);

  type item = {
    title: string,
    command: string,
    parentId: uniqueId,
  };

  type menu = {
    order: int,
    uniqueId,
    title: string,
  };

  let idToString = uniqueId => uniqueId |> String.concat(".");

  let item = (~title, ~command, ~parent) => {
    title,
    command,
    parentId: parent.uniqueId,
  };

  let command = (~parent, command: Command.t(_)) => {
    command: command.id,
    title: command.title |> Option.value(~default="(null)"),
    parentId: parent.uniqueId,
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
    items: StringMap.t(list(item)),
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

  let items = items => {
    let itemsMap =
      items
      |> List.fold_left(
           (acc, curr: item) => {
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
    let merge = (_key, maybeA, maybeB) =>
      switch (maybeA, maybeB) {
      | (Some(_) as a, _) => a
      | (None, Some(_) as b) => b
      | (None, None) => None
      };
    let items' = StringMap.merge(merge, map1.items, map2.items);

    let menus' = StringMap.merge(merge, map1.menus, map2.menus);

    {menus: menus', items: items'};
  };

  let ofList = schemas => {
    schemas |> List.fold_left((acc, curr) => {union(acc, curr)}, initial);
  };
};

type t = Schema.t;

module Item = {
  type t = Schema.item;

  let title = ({title, _}: Schema.item) => title;

  let command = ({command, _}: Schema.item) => command;
};

type builtMenu = {schema: t};

module Menu = {
  type t = Schema.menu;

  type contentItem =
    | Item(Item.t);

  let title = ({title, _}: Schema.menu) => title;
  let uniqueId = ({uniqueId, _}: Schema.menu) =>
    uniqueId |> Schema.idToString;

  let contents = (menu: t, builtMenu) => {
    StringMap.find_opt(uniqueId(menu), builtMenu.schema.items)
    |> Option.value(~default=[])
    |> List.map(item => Item(item));
  };
};

let build = (~contextKeys as _, ~commands as _, menu) => {schema: menu};

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
