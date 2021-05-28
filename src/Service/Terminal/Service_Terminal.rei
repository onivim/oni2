open Oni_Core;

[@deriving show({with_path: false})]
type msg =
  | ProcessExit({
      id: int,
      exitCode: int,
    })
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
      screen: EditorTerminal.Screen.t,
      cursor: EditorTerminal.Cursor.t,
    });

module Sub: {
  let terminal:
    (
      ~setup: Setup.t,
      ~id: int,
      ~launchConfig: Exthost.ShellLaunchConfig.t,
      ~columns: int,
      ~rows: int,
      ~workspaceUri: Uri.t,
      ~extHostClient: Exthost.Client.t
    ) =>
    Isolinear.Sub.t(msg);
};

module Effect: {
  let input: (~id: int, string) => Isolinear.Effect.t(msg);
  let paste: (~id: int, string) => Isolinear.Effect.t(msg);
};

let handleExtensionMessage: Exthost.Msg.TerminalService.msg => unit;

let getScreen: int => option(EditorTerminal.Screen.t);
