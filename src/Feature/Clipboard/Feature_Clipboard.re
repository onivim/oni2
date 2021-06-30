open Oni_Core.Utility;

type model = unit;
let initial = ();

[@deriving show]
type command =
  | PasteAction;

[@deriving show]
type msg =
  | Command(command)
  | PasteClipboardEmpty
  | PasteClipboardText({text: string});

module Msg = {
  let paste = Command(PasteAction);
};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | Pasted({
      rawText: string,
      isMultiLine: bool,
      lines: array(string),
    });

module Effects = {
  let retrieveClipboard = () => {
    let toMsg =
      fun
      | None => PasteClipboardEmpty
      | Some(text) => PasteClipboardText({text: text});

    Service_Clipboard.Effects.getClipboardText(~toMsg);
  };
};

let update = (msg, model) => {
  switch (msg) {
  | Command(PasteAction) => (model, Effect(Effects.retrieveClipboard()))
  // TODO: Show an error message in this case?
  | PasteClipboardEmpty => (model, Nothing)
  | PasteClipboardText({text}) =>
    let (isMultiLine, lines) = StringEx.splitLines(text);
    (
      model,
      Pasted({
        isMultiLine,
        lines,
        rawText: StringEx.removeWindowsNewLines(text),
      }),
    );
  };
};

module Commands = {
  open Feature_Commands.Schema;
  let paste =
    define(
      ~category="Clipboard",
      ~title="Paste into editor",
      "editor.action.clipboardPasteAction",
      Command(PasteAction),
    );
};

module MenuItems = {
  open Oni_Core.ContextMenu.Schema;

  let paste = command(~title="Paste", Commands.paste);
};

module Keybindings = {
  open Feature_Input.Schema;
  let pasteNonMac =
    bind(
      ~key="<C-V>",
      ~command=Commands.paste.id,
      // The WhenExpr parser doesn't support precedence, so we manually construct it here...
      // It'd be nice to bring back explicit precedence via '(' and ')'
      // Alternatively, a manual construction could be done with separate bindings for !isMac OR each condition
      ~condition=
        WhenExpr.(
          And([
            Not(Defined("isMac")),
            Or([
              And([Defined("editorTextFocus"), Defined("insertMode")]),
              Defined("textInputFocus"),
              Defined("commandLineFocus"),
            ]),
          ])
        ),
    );

  let pasteMac =
    bind(
      ~key="<D-V>",
      ~command=Commands.paste.id,
      ~condition="isMac" |> WhenExpr.parse,
    );
};

module Contributions = {
  let commands = [Commands.paste];

  let keybindings = Keybindings.[pasteMac, pasteNonMac];

  module MenuItems = {
    let all = MenuItems.[paste];
    let paste = MenuItems.paste;
  };
};
