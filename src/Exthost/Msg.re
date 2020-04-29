[@deriving show]
type t =
  | Connected
  | Ready
  | Commands(Commands.msg)
  | DebugService(DebugService.msg)
  | ExtensionService(ExtensionService.msg)
  | MessageService(MessageService.msg)
  | StatusBar(StatusBar.msg)
  | Telemetry(Telemetry.msg)
  | TerminalService(TerminalService.msg)
  | Initialized
  | Disconnected
  | Unhandled
  | Unknown({
      method: string,
      args: Yojson.Safe.t,
    });
