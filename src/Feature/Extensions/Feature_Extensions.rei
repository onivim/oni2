open Oni_Core;
open Exthost.Extension;

type model;

[@deriving show({with_path: false})]
type msg;

module Msg: {
  let exthost: Exthost.Msg.ExtensionService.msg => msg;
  let storage:
    (~resolver: Lwt.u(Exthost.Reply.t), Exthost.Msg.Storage.msg) => msg;

  let languages:
    (~resolver: Lwt.u(Exthost.Reply.t), Exthost.Msg.Languages.msg) => msg;
  let discovered: list(Scanner.ScanResult.t) => msg;
  let keyPressed: string => msg;
  let pasted: string => msg;

  let command: (~command: string, ~arguments: list(Yojson.Safe.t)) => msg;
};

type outmsg =
  | Nothing
  | Focus
  | Effect(Isolinear.Effect.t(msg))
  | NewExtensions(list(Scanner.ScanResult.t))
  | InstallSucceeded({
      extensionId: string,
      contributions: Exthost.Extension.Contributions.t,
    })
  | NotifySuccess(string)
  | NotifyFailure(string)
  | OpenExtensionDetails
  | SelectTheme({themes: list(Exthost.Extension.Contributions.Theme.t)})
  | UnhandledWindowMovement(Component_VimWindows.outmsg);

let pick: (Exthost.Extension.Manifest.t => 'a, model) => list('a);

let themeById: (~id: string, model) => option(Contributions.Theme.t);
let themesByName: (~filter: string, model) => list(string);

let snippetFilePaths:
  (~fileType: string, model) => list(FpExp.t(FpExp.absolute));

// [hasCompletedDiscovery(model)] returns [true] if all
// local extensions have been identified.
let hasCompletedDiscovery: model => bool;

let isBusy: model => bool;
let isSearchInProgress: model => bool;

let isInstalled: (~extensionId: string, model) => bool;
let isInstalling: (~extensionId: string, model) => bool;
let isUninstalling: (~extensionId: string, model) => bool;

let update: (~extHostClient: Exthost.Client.t, msg, model) => (model, outmsg);

let resetFocus: model => model;

let all: model => list(Scanner.ScanResult.t);
let activatedIds: model => list(string);

let menus: model => list(Menu.Schema.definition);
let sub:
  (~isVisible: bool, ~setup: Oni_Core.Setup.t, model) => Isolinear.Sub.t(msg);

module Persistence: {
  type t = Yojson.Safe.t;
  let initial: t;

  let codec: Oni_Core.Persistence.Schema.Codec.t(t);

  let get: (~shared: bool, model) => t;
};

let initial:
  (
    ~workspacePersistence: Persistence.t,
    ~globalPersistence: Persistence.t,
    ~extensionsFolder: option(FpExp.t(FpExp.absolute))
  ) =>
  model;

module ListView: {
  let make:
    (
      ~key: Brisk_reconciler.Key.t=?,
      ~model: model,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~isFocused: bool,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};

module DetailsView: {
  let make:
    (
      ~model: model,
      ~theme: ColorTheme.Colors.t,
      ~tokenTheme: Oni_Syntax.TokenTheme.t,
      ~font: UiFont.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};

module Contributions: {
  let commands: (~isFocused: bool, model) => list(Command.t(msg));
  let contextKeys: (~isFocused: bool, model) => WhenExpr.ContextKeys.t;
};
