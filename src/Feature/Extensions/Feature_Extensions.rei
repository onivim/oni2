open Oni_Core;
open Exthost.Extension;

type model;

[@deriving show({with_path: false})]
type msg;

module Msg: {
  let discovered: list(Scanner.ScanResult.t) => msg;
  let exthost: Exthost.Msg.ExtensionService.msg => msg;
  let keyPressed: string => msg;
  let pasted: string => msg;
};

type outmsg =
  | Nothing
  | Focus
  | Effect(Isolinear.Effect.t(msg))
  | NotifySuccess(string)
  | NotifyFailure(string)
  | OpenExtensionDetails;

let initial: (~extensionsFolder: option(string)) => model;

let isBusy: model => bool;
let isSearchInProgress: model => bool;

let update: (~extHostClient: Exthost.Client.t, msg, model) => (model, outmsg);

let all: model => list(Scanner.ScanResult.t);
let activatedIds: model => list(string);

let menus: model => list(Menu.Schema.definition);
let commands: model => list(Command.t(msg));

let sub: (~setup: Oni_Core.Setup.t, model) => Isolinear.Sub.t(msg);

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
      ~font: UiFont.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
