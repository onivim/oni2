open Kernel;

module Schema = {
  type item = {
    title: string,
    command: string,
    parentId: string,
    uniqueId: string,
  };

  type menu = {
    uniqueId: string,
    title: string,
  };

  let item = (~title, ~command, ~parent) => {
    title,
    command,
    parentId: parent.uniqueId,
    uniqueId: parent.uniqueId ++ "." ++ title,
  };

  let menu = (~uniqueId, ~title, ~parent: option(menu)) => {
    let uniqueId =
      switch (parent) {
      | None => uniqueId
      | Some(parent) => parent.uniqueId ++ "." ++ uniqueId
      };

    {uniqueId, title};
  };

  type t = {
    menus: StringMap.t(menu),
    items: list(item),
  };

  let initial = {
    menus: StringMap.empty,
    items: [],
  }

  let menus = (menus) => {
    let menuMap =
      menus
      |> List.fold_left(
           (acc, curr: menu) => {acc |> StringMap.add(curr.uniqueId, curr)},
           StringMap.empty,
         );

    {menus: menuMap, items: []};
    
  }

  let items = (items) => {
    menus: StringMap.empty,
    items: []
  };

  let union = (map1, map2) => {
    let items' = map1.items @ map2.items;

    let menus' =
      StringMap.merge(
        (key, maybeA, maybeB) => {
          switch (maybeA, maybeB) {
          | (Some(_) as a, _) => a
          | (None, Some(_) as b) => b
          | (None, None) => None
          }
        },
        map1.menus,
        map2.menus,
      );

    {menus: menus', items: items'};
  };

  let ofList = (schemas) => {
    schemas
    |> List.fold_left((acc, curr) => {
      union(acc, curr)
    }, initial);
  }
};

type t = Schema.t;

let initial = menu => menu;

module Item = {
  type t = Schema.item;

  let title = ({title, _}: Schema.item) => title;

  let command = ({command, _}: Schema.item) => command;
}

module Menu = {
  type t = Schema.menu;

  type contentItem = 
  | SubMenu(t)
  | Item(Item.t)

  let title = ({title, _}: Schema.menu) => title;

  let contents = (_, _) =>  [];
}

type builtMenu = {
  schema: t
};

let build = (~contextKeys as _, ~commands as _, menu) => {
  schema: menu
};

let top = ({schema, _}) => {
  Schema.({
    schema.menus
    |> StringMap.bindings
    |> List.map(snd)
  })
}
