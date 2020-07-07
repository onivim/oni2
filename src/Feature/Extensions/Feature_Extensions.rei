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
  | SearchQueryResults(Service_Extensions.Query.t)
  | SearchQueryError(string)
  | SearchText(Feature_InputText.msg);


type outmsg =
  | Nothing
  | Focus
  | Effect(Isolinear.Effect.t(msg));

let initial: model;

let update: (~extHostClient: Exthost.Client.t, msg, model) => (model, outmsg);

let all: model => list(Scanner.ScanResult.t);
let activatedIds: model => list(string);

let menus: model => list(Menu.Schema.definition);
let commands: model => list(Command.t(msg));

let sub: (~setup: Oni_Core.Setup.t, model) => Isolinear.Sub.t(msg);

module ListView: {
  let make:
    (
      ~model: model,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~isFocused: bool,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
