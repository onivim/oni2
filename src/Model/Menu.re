open Actions;

type t = {
  variant,
  text: string,
  prefix: option(string),
  cursorPosition: int,
  items: array(menuItem),
  filterProgress: progress,
  ripgrepProgress: progress,
  selected: option(int) // TODO: Might not be a great idea to use an index to refer to a specific item in an array that changes over time
}

and variant =
  Actions.menuVariant =
    | CommandPalette
    | Buffers
    | WorkspaceFiles
    | Wildmenu(Vim.Types.cmdlineType)
    | Themes;

let defaults = variant => {
  variant,
  text: "",
  prefix: None,
  cursorPosition: 0,
  selected: None,
  items: [||],
  filterProgress: Complete,
  ripgrepProgress: Complete,
};

// TODO: This doesn't really belong here. Find a better home for it.
let getLabel = (item: menuItem) => {
  switch (item.category) {
  | Some(v) => v ++ ": " ++ item.name
  | None => item.name
  };
};
