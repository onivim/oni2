open Oni_Core;

// MODEL

[@deriving show]
type msg;

module Msg: {
    let keyPressed: string => msg;
};

type model;

let initial: model;

//let getFileIcon: (
//    Exthost.LanguageInfo.t,
//    IconTheme.t,
//    string
//) => option(IconTheme.IconDefinition.t)
//
//let getDirectoryTree:
//    (string, Exthost.LanguageInfo.t, IconTheme.t, list(string)) => FsTreeNode.t;

// UPDATE

type outmsg =
| Nothing
| Effect(Isolinear.Effect.t(msg))
| OpenFile(string)
| GrabFocus;

let update: (
    ~configuration: Oni_Core.Configuration.t, 
    ~languageInfo: Exthost.LanguageInfo.t,
    ~iconTheme: Oni_Core.IconTheme.t,
msg, model) => (model, outmsg);

module View: {
    let make: (
        ~model: model,
        ~theme: ColorTheme.Colors.t,
        ~font: UiFont.t,
        ~dispatch: msg => unit,
        unit,
    ) => Revery.UI.element;
}
