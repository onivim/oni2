open Oni_Core;

// Placeholder model type
type model = int;
let initial: model = 0;

[@deriving show]
type command =
  | ShowEmojiAndSymbols;

[@deriving show]
type msg =
  | Noop
  | Command(command);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

module Commands = {
  open Feature_Commands.Schema;

  let showEmojiAndSymbols =
    define(
      ~category="Keyboard",
      ~title="Input Emoji & Symbols",
      ~isEnabledWhen="isMac" |> WhenExpr.parse,
      "oni2.keyboard.emojiAndSymbols", // This doesn't exist in VSCode
      Command(ShowEmojiAndSymbols),
    );
};

module MenuItems = {
  open ContextMenu.Schema;
  module Edit = {
    let emojiAndSymbols =
      command(~title="Emoji & Symbols", Commands.showEmojiAndSymbols);
  };
};

module Keybindings = {
  open Feature_Input.Schema;

  let emojiAndSymbols =
    bind(
      ~key="<C-D-Space>",
      ~command=Commands.showEmojiAndSymbols.id,
      ~condition="isMac" |> WhenExpr.parse,
    );
};

module Contributions = {
  let commands = Commands.[showEmojiAndSymbols];

  let menuGroups =
    ContextMenu.Schema.[
      group(
        ~order=600,
        ~parent=Feature_MenuBar.Global.edit,
        MenuItems.Edit.[emojiAndSymbols],
      ),
    ];

  let keybindings = Keybindings.[emojiAndSymbols];
};

let update = (model, msg) =>
  switch (msg) {
  | Command(ShowEmojiAndSymbols) => (
      model,
      Effect(
        Service_Keyboard.Effects.showEmojiAndSymbols
        |> Isolinear.Effect.map(_ => Noop),
      ),
    )
  | Noop => (model, Nothing)
  };
