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
    (model, Pasted({isMultiLine, lines, rawText: text}));
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

module Contributions = {
  let commands = [Commands.paste];
};
