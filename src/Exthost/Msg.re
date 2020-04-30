module DocumentContentProvider = {
  [@deriving show]
  type msg =
    | RegisterTextContentProvider({
        handle: int,
        scheme: string
    })
    | UnregisterTextContentProvider({
      handle: int,
    })
    | VirtualDocumentChange({
      uri: Oni_Core.Uri.t,
      value: string,
    });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$registerTextContentProvider", `List([`Int(handle), `String(scheme)])) =>
      Ok(RegisterTextContentProvider({ handle, scheme }))
    | ("$unregisterTextContentProvider", `List([`Int(handle)])) =>
      Ok(UnregisterTextContentProvider({ handle: handle }))
    | ("$unregisterTextContentProvider", `List([uriJson, `String(value)])) =>
      
      uriJson
      |> Oni_Core.Uri.of_yojson
      |> Result.map(uri => {
        VirtualDocumentChange({ uri, value }) 
      });
    | _ => Error("Unhandled method: " ++ method)
    };
  };
}

[@deriving show]
type t =
  | Connected
  | Ready
  | Commands(Commands.msg)
  | DebugService(DebugService.msg)
  | DocumentContentProvider(DocumentContentProvider.msg)
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
