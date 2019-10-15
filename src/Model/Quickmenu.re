open Actions

type t = {
  variant: variant,
  text: string,
  prefix: option(string),
  cursorPosition: int,
  source: menuSource,
  selected: int // TODO: Might not be a great idea to use an index to refer to a specific item in an array that changes over time
}

and variant = Actions.menuVariant =
  | CommandPalette
  | Buffers
  | WorkspaceFiles
  | Wildmenu(Vim.Types.cmdlineType)

let defaults = variant => {
  variant,
  text: "",
  prefix: None,
  cursorPosition: 0,
  selected: 0,
  source: Loading
};


let getCount = fun
  | Loading =>
    0

  | Progress({ items })
  | Complete(items) =>
    Array.length(items);


let getItems = fun
  | Loading =>
    [||]

  | Progress({ items })
  | Complete(items) =>
    items