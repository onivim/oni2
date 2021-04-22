open Oni_Core;

// MODEL

[@deriving show]
type command = Exthost.SCM.command;

module Resource: {
  [@deriving show]
  type t = {
    handle: int,
    uri: Uri.t,
    //    icons: Exthost.SCM.Resource.Icons.t,
    tooltip: string,
    strikeThrough: bool,
    faded: bool,
  };
};

module ResourceGroup: {
  [@deriving show]
  type t;
};

module Provider: {
  [@deriving show]
  type t = {
    handle: int,
    id: string,
    label: string,
    rootUri: option(Uri.t),
    resourceGroups: list(ResourceGroup.t),
    hasQuickDiffProvider: bool,
    count: int,
    commitTemplate: string,
    acceptInputCommand: option(command),
    inputVisible: bool,
    validationEnabled: bool,
    statusBarCommands: list(command),
  };
};

[@deriving show]
type model;

let resetFocus: model => model;

let count: model => int;

let initial: model;

let statusBarCommands: (~workingDirectory: string, model) => list(command);

// UPDATE

[@deriving show]
type msg;

module Msg: {
  let keyPressed: string => msg;
  let paste: string => msg;
  let documentContentProvider: Exthost.Msg.DocumentContentProvider.msg => msg;
};

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | EffectAndFocus(Isolinear.Effect.t(msg))
  | Focus
  | OpenFile(string)
  | PreviewFile(string)
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | OriginalContentLoaded({
      bufferId: int,
      originalLines: array(string),
    })
  | Nothing;

let update:
  (
    ~previewEnabled: bool,
    ~fileSystem: Feature_FileSystem.model,
    Exthost.Client.t,
    model,
    msg
  ) =>
  (model, outmsg);

let handleExtensionMessage:
  (~dispatch: msg => unit, Exthost.Msg.SCM.msg) => unit;

// SUBSCRIPTION

let sub:
  (~activeBuffer: Oni_Core.Buffer.t, ~client: Exthost.Client.t, model) =>
  Isolinear.Sub.t(msg);

// VIEW

module Pane: {
  let make:
    (
      ~key: Brisk_reconciler.Key.t=?,
      ~model: model,
      ~workingDirectory: string,
      ~isFocused: bool,
      ~iconTheme: Oni_Core.IconTheme.t,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~theme: ColorTheme.Colors.t,
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
