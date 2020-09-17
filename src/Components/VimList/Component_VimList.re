// MODEL

[@deriving show]
type model('item) = {
  scrollY: float,
  rowHeight: int,
  items: list('item),
  focused: option(int),
};

let create = (~rowHeight: int, items) => {
  scrollY: 0.,
  rowHeight,
  items: items,
  focused: None,
};

[@deriving show]
type command =
  | CursorToTop //gg
  | CursorToBottom // G
  | CursorDown // j
  | CursorUp // k
  | ScrollCursorCenter // zz
  | ScrollCursorBottom // zb
  | ScrollCursorTop; // zt

[@deriving show]
type msg =
  | Command(command);

type outmsg =
  | Nothing;

let set = (items, model) => {
  ...model,
  items
};

// UPDATE

let update = (msg, model) => {
  (model, Nothing);
};

// CONTRIBUTIONS

module Commands = {
  open Feature_Commands.Schema;

  let gg = define("vim.list.gg", Command(CursorToTop));
  let g = define("vim.list.G", Command(CursorToBottom));
  let j = define("vim.list.j", Command(CursorDown));
  let k = define("vim.list.k", Command(CursorUp));
  let zz = define("vim.list.zz", Command(ScrollCursorCenter));
  let zb = define("vim.list.zb", Command(ScrollCursorBottom));
  let zt = define("vim.list.zt", Command(ScrollCursorTop));
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let vimListNavigation = bool("vimListNavigation", _ => true);
};

module Keybindings = {
  open Oni_Input;

  let commandCondition = "vimListNavigation" |> WhenExpr.parse;

  let keybindings =
    Keybindings.[
      {
        key: "gg",
        command: Commands.gg.id,
        condition: commandCondition,
      },
      {
        key: "G",
        command: Commands.g.id,
        condition: commandCondition,
      },
      {
        key: "j",
        command: Commands.j.id,
        condition: commandCondition,
      },
      {
        key: "k",
        command: Commands.k.id,
        condition: commandCondition,
      },
      {
        key: "zz",
        command: Commands.zz.id,
        condition: commandCondition,
      },
      {
        key: "zb",
        command: Commands.zb.id,
        condition: commandCondition,
      },
      {
        key: "zt",
        command: Commands.zt.id,
        condition: commandCondition,
      },
    ];
};

module Contributions = {
  let contextKeys = ContextKeys.[vimListNavigation];

  let keybindings = Keybindings.keybindings;

  let commands = Commands.[
    gg,
    g,
    j,
    k,
    zz,
    zb,
    zt
  ];
};

module View = {
  let make = (
    ~items,
    ~uniqueId,
    ~focused,
    ~scrollY,
    ~dispatch,
    ~rowHeight,
    ~render, 
    ()
  ) => Revery.UI.React.empty;
}
