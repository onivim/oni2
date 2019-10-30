open Actions

type t = {
  variant: variant,
  text: string,
  prefix: option(string),
  cursorPosition: int,
  source: menuSource,
  selected: option(int) // TODO: Might not be a great idea to use an index to refer to a specific item in an array that changes over time
}

and variant = Actions.menuVariant =
  | CommandPalette
  | Buffers
  | WorkspaceFiles
  | Wildmenu(Vim.Types.cmdlineType)
  | Themes

let defaults = variant => {
  variant,
  text: "",
  prefix: None,
  cursorPosition: 0,
  selected: None,
  source: Loading
};


let getCount = fun
  | Loading =>
    0

  | Progress({ items, _ })
  | Complete(items) =>
    Array.length(items);


let getItems = fun
  | Loading =>
    [||]

  | Progress({ items, _ })
  | Complete(items) =>
    items


// TODO: This doesn't really belong here. Find a better home for it.
let getLabel = (item: menuItem) => {
  switch (item.category) {
  | Some(v) => v ++ ": " ++ item.name
  | None => item.name
  };
}; 