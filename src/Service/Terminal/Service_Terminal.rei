open Oni_Core;

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
      ~extHostClient: Exthost.Client.t
    ) =>
    Isolinear.Sub.t(msg);
};

module Effect: {let input: (~id: int, string) => Isolinear.Effect.t(msg);};

let handleExtensionMessage: Exthost.Msg.TerminalService.msg => unit;

let getScreen: int => option(ReveryTerminal.Screen.t);
