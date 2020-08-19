open Oni_Core;

// MODEL

[@deriving show]
type command;

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
  type t = {
    handle: int,
    id: string,
    label: string,
    hideWhenEmpty: bool,
    resources: list(Resource.t),
  };
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
    statusBarCommands: list(Exthost.Command.t),
  };
};

[@deriving show]
type model;

let initial: model;

let statusBarCommands: model => list(Exthost.Command.t);

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
  | Focus
  | Nothing;

let update: (Exthost.Client.t, model, msg) => (model, outmsg);

let getOriginalLines: (Oni_Core.Buffer.t, model) => option(array(string));
let setOriginalLines: (Oni_Core.Buffer.t, array(string), model) => model;

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
      ~onItemClick: Resource.t => unit,
      ~isFocused: bool,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
