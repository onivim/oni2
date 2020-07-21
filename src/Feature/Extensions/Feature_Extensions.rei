open Oni_Core;
open Exthost.Extension;

type model;

[@deriving show({with_path: false})]
type msg =
  | Activated(string /* id */)
  | Discovered([@opaque] list(Scanner.ScanResult.t))
  | ExecuteCommand({
      command: string,
      arguments: [@opaque] list(Json.t),
    })
  | KeyPressed(string)
  | Pasted(string)
  | SearchQueryResults(Service_Extensions.Query.t)
  | SearchQueryError(string)
  | SearchText(Feature_InputText.msg)
  | UninstallExtensionClicked({extensionId: string})
  | UninstallExtensionSuccess({extensionId: string})
  | UninstallExtensionFailed({
      extensionId: string,
      errorMsg: string,
    })
  | InstallExtensionClicked({extensionId: string})
  | InstallExtensionSuccess({
      extensionId: string,
      scanResult: [@opaque] Scanner.ScanResult.t,
    })
  | InstallExtensionFailed({
      extensionId: string,
      errorMsg: string,
    });

type outmsg =
  | Nothing
  | Focus
  | Effect(Isolinear.Effect.t(msg))
  | InstallSucceeded({
      extensionId: string,
      contributions: Exthost.Extension.Contributions.t,
    })
  | NotifySuccess(string)
  | NotifyFailure(string);

let pick: (Exthost.Extension.Manifest.t => 'a, model) => list('a);

let themeByName: (~name: string, model) => option(Contributions.Theme.t);

let initial: (~extensionsFolder: option(string)) => model;

let isBusy: model => bool;
let isSearchInProgress: model => bool;

let isInstalling: (~extensionId: string, model) => bool;
let isUninstalling: (~extensionId: string, model) => bool;

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
