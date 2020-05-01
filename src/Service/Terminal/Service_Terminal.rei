open Oni_Core;
open Oni_Extensions;

[@deriving show({with_path: false})]
type msg =
  | ProcessStarted({
      id: int,
      pid: int,
    })
  | ProcessTitleChanged({
      id: int,
      title: string,
    })
  | ScreenUpdated({
      id: int,
      screen: ReveryTerminal.Screen.t,
    })
  | CursorMoved({
      id: int,
      cursor: ReveryTerminal.Cursor.t,
    });

module Sub: {
  let terminal:
    (
      ~id: int,
      ~arguments: list(string),
      ~cmd: string,
      ~columns: int,
      ~rows: int,
      ~workspaceUri: Uri.t,
      ~extHostClient: ExtHostClient.t
    ) =>
    Isolinear.Sub.t(msg);
};

module Effect: {let input: (~id: int, string) => Isolinear.Effect.t(msg);};

let handleExtensionMessage: ExtHostClient.Terminal.msg => unit;

let getScreen: int => option(ReveryTerminal.Screen.t);
