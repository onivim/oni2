open Oni_Core;

module Decorations = {
  [@deriving show]
  type msg =
    | RegisterDecorationProvider({
        handle: int,
        label: string,
      })
    | UnregisterDecorationProvider({handle: int})
    | DecorationsDidChange({
        handle: int,
        uris: list(Uri.t),
      });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | (
        "$registerDecorationProvider",
        `List([`Int(handle), `String(label)]),
      ) =>
      Ok(RegisterDecorationProvider({handle, label}))

    | ("$unregisterDecorationProvider", `List([`Int(handle)])) =>
      Ok(UnregisterDecorationProvider({handle: handle}))

    | ("$onDidChange", `List([`Int(handle), `List(resources)])) =>
      let uris =
        resources
        |> List.filter_map(json =>
             Uri.of_yojson(json) |> Stdlib.Result.to_option
           );
      Ok(DecorationsDidChange({handle, uris}));
    | _ => Error("Unhandled method: " ++ method)
    };
  };
};

module Diagnostics = {
  [@deriving (show, yojson({strict: false}))]
  type entries = (Uri.t, [@opaque] list(Diagnostic.t));
  [@deriving show]
  type msg =
    | ChangeMany({
        owner: string,
        entries,
      })
    | Clear({owner: string});

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$changeMany", `List([`String(owner), diagnosticsJson])) =>
      diagnosticsJson
      |> entries_of_yojson
      |> Result.map(entries => {ChangeMany({owner, entries})})
    | ("$clear", `List([`String(owner)])) => Ok(Clear({owner: owner}))
    | _ => Error("Unhandled method: " ++ method)
    };
  };
};

module DocumentContentProvider = {
  [@deriving show]
  type msg =
    | RegisterTextContentProvider({
        handle: int,
        scheme: string,
      })
    | UnregisterTextContentProvider({handle: int})
    | VirtualDocumentChange({
        uri: Uri.t,
        value: string,
      });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | (
        "$registerTextContentProvider",
        `List([`Int(handle), `String(scheme)]),
      ) =>
      Ok(RegisterTextContentProvider({handle, scheme}))
    | ("$unregisterTextContentProvider", `List([`Int(handle)])) =>
      Ok(UnregisterTextContentProvider({handle: handle}))
    | ("$unregisterTextContentProvider", `List([uriJson, `String(value)])) =>
      uriJson
      |> Uri.of_yojson
      |> Result.map(uri => {VirtualDocumentChange({uri, value})})
    | _ => Error("Unhandled method: " ++ method)
    };
  };
};

[@deriving show]
type t =
  | Connected
  | Ready
  | Commands(Commands.msg)
  | DebugService(DebugService.msg)
  | Decorations(Decorations.msg)
  | Diagnostics(Diagnostics.msg)
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
