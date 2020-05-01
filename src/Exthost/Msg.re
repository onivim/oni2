open Oni_Core;
open Oni_Core.Utility;

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
  [@deriving show]
  type entry = (Uri.t, [@opaque] list(Diagnostic.t));

  let decodeEntry = json =>
    switch (json) {
    | `List([uriJson, diagnosticListJson]) =>
      uriJson
      |> Uri.of_yojson
      |> ResultEx.flatMap(uri => {
           diagnosticListJson
           |> Yojson.Safe.Util.to_list
           |> List.map(Json.Decode.decode_value(Diagnostic.decode))
           |> Base.Result.all
           |> Result.map(diagList => (uri, diagList))
           |> Result.map_error(Json.Decode.string_of_error)
         })
    | _ => Error("Expected 2-element tuple")
    };

  [@deriving show]
  type msg =
    | ChangeMany({
        owner: string,
        entries: list(entry),
      })
    | Clear({owner: string});

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$changeMany", `List([`String(owner), `List(diagnosticsJson)])) =>
      diagnosticsJson
      |> List.map(decodeEntry)
      |> Base.Result.all
      |> Result.map(entries => ChangeMany({owner, entries}))
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
