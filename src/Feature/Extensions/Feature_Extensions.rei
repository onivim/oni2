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
    });

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let empty: model;

let update: (~extHostClient: Exthost.Client.t, msg, model) => (model, outmsg);

let all: model => list(Scanner.ScanResult.t);
let activatedIds: model => list(string);

let menus: model => list(Menu.Schema.definition);
let commands: model => list(Command.t(msg));

module ListView: {
  let make:
    (
      ~key: Brisk_reconciler.Key.t=?,
      ~model: model,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      unit
    ) =>
    Revery.UI.element;
};
