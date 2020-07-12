type model = unit;

type command = 
| PasteAction;

type msg = 
| Command(command);

type outmsg = 
| Nothing
| Effect(Isolinear.Effect.t(msg))
| Pasted({ lines: list(string)  });

module Commands = {
   open Feature_Commands.Schema;
   let paste = 
    define(
        ~category="Clipboard",
        ~title="Paste into editor",
        "editor.action.clipboardPasteAction",
        Command(PasteAction);
    )
};
