// MODEL

[@deriving show]
type model = unit;

let initial = ();

[@deriving show]
type command =
  | MoveLeft
  | MoveRight
  | MoveUp
  | MoveDown
  | PreviousTab
  | NextTab;

[@deriving show]
type msg =
  | Command(command);

type outmsg =
  | Nothing
  | FocusLeft
  | FocusRight
  | FocusUp
  | FocusDown
  | PreviousTab
  | NextTab;

// UPDATE

let update = (msg, model) => {
  switch (msg) {
  | Command(command) =>
    switch (command) {
    | MoveLeft => (model, FocusLeft)
    | MoveRight => (model, FocusRight)
    | MoveUp => (model, FocusUp)
    | MoveDown => (model, FocusDown)
    | PreviousTab => (model, PreviousTab)
    | NextTab => (model, NextTab)
    }
  };
};

// CONTRIBUTIONS

module Commands = {
  open Feature_Commands.Schema;

  let moveLeft = define("vim.window.moveLeft", Command(MoveLeft));
  let moveRight = define("vim.window.moveRight", Command(MoveRight));
  let moveUp = define("vim.window.moveUp", Command(MoveUp));
  let moveDown = define("vim.window.moveDown", Command(MoveDown));
  let previousTab = define("vim.window.previousTab", Command(PreviousTab));
  let nextTab = define("vim.window.previousTab", Command(NextTab));
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let vimWindowNavigation = bool("vimWindowNavigation", _ => true);
};

module Keybindings = {
  open Oni_Input;

  let commandCondition = "vimWindowNavigation" |> WhenExpr.parse;

  let noTextInputCondition =
    "!textInputFocus && vimWindowNavigation" |> WhenExpr.parse;

  let keybindings =
    Keybindings.[
      {
        key: "<C-W>H",
        command: Commands.moveLeft.id,
        condition: commandCondition,
      },
      {
        key: "<C-W><C-H>",
        command: Commands.moveLeft.id,
        condition: commandCondition,
      },
      {
        key: "<C-W><LEFT>",
        command: Commands.moveLeft.id,
        condition: commandCondition,
      },
      {
        key: "<C-W>L",
        command: Commands.moveRight.id,
        condition: commandCondition,
      },
      {
        key: "<C-W><C-L>",
        command: Commands.moveRight.id,
        condition: commandCondition,
      },
      {
        key: "<C-W><RIGHT>",
        command: Commands.moveRight.id,
        condition: commandCondition,
      },
      {
        key: "<C-W>K",
        command: Commands.moveUp.id,
        condition: commandCondition,
      },
      {
        key: "<C-W><C-K>",
        command: Commands.moveUp.id,
        condition: commandCondition,
      },
      {
        key: "<C-W><UP>",
        command: Commands.moveUp.id,
        condition: commandCondition,
      },
      {
        key: "<C-W>J",
        command: Commands.moveDown.id,
        condition: commandCondition,
      },
      {
        key: "<C-W><C-J>",
        command: Commands.moveDown.id,
        condition: commandCondition,
      },
      {
        key: "<C-W><DOWN>",
        command: Commands.moveDown.id,
        condition: commandCondition,
      },
      {
        key: "gt",
        command: Commands.nextTab.id,
        condition: noTextInputCondition,
      },
      {
        key: "gT",
        command: Commands.previousTab.id,
        condition: noTextInputCondition,
      },
    ];
};

module Contributions = {
  let contextKeys = model => {
    WhenExpr.ContextKeys.(
      ContextKeys.[vimWindowNavigation]
      |> Schema.fromList
      |> fromSchema(model)
    );
  };

  let keybindings = Keybindings.keybindings;

  let commands =
    Commands.[moveLeft, moveDown, moveUp, moveRight, previousTab, nextTab];
};
