open EditorCoreTypes;
open Oni_Core;
open Revery.UI;

type model;

let initial: model;

[@deriving show]
type msg;

module Msg: {
  let input: string => msg;
  let pasted: string => msg;
};

type outmsg =
  | OpenFile({
      filePath: string,
      location: CharacterPosition.t,
    })
  | PreviewFile({
      filePath: string,
      location: CharacterPosition.t,
    })
  | Focus
  | UnhandledWindowMovement(Component_VimWindows.outmsg);

let update: (~previewEnabled: bool, model, msg) => (model, option(outmsg));

let resetFocus: (~query: option(string), model) => model;

let sub:
  (
    ~config: Oni_Core.Config.resolver,
    ~workingDirectory: string,
    ~setup: Setup.t,
    model
  ) =>
  Isolinear.Sub.t(msg);

let make:
  (
    ~config: Oni_Core.Config.resolver,
    ~theme: ColorTheme.Colors.t,
    ~uiFont: UiFont.t,
    ~iconTheme: IconTheme.t,
    ~languageInfo: Exthost.LanguageInfo.t,
    ~isFocused: bool,
    ~model: model,
    ~dispatch: msg => unit,
    ~workingDirectory: string,
    unit
  ) =>
  React.element(React.node);

module Contributions: {
  let commands: (~isFocused: bool) => list(Command.t(msg));
  let contextKeys: (~isFocused: bool, model) => WhenExpr.ContextKeys.t;
  let configuration: list(Config.Schema.spec);
  let keybindings: list(Feature_Input.Schema.keybinding);
};
