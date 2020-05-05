open Oni_Core;
open Oni_Core.Utility;
module Commands = {
  [@deriving show]
  type msg =
    | RegisterCommand(string)
    | UnregisterCommand(string)
    | ExecuteCommand({
        command: string,
        args: list(Yojson.Safe.t),
        retry: bool,
      })
    | GetCommands;

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$registerCommand", `List([`String(cmd)])) =>
      Ok(RegisterCommand(cmd))
    | ("$unregisterCommand", `List([`String(cmd)])) =>
      Ok(UnregisterCommand(cmd))
    | ("$getCommands", _) => Ok(GetCommands)
    | (
        "$executeCommand",
        `List([`String(command), `List(args), `Bool(retry)]),
      ) =>
      Ok(ExecuteCommand({command, args, retry}))
    | _ => Error("Unhandled method: " ++ method)
    };
  };
};

module DebugService = {
  [@deriving show]
  type msg =
    | RegisterDebugTypes(list(string));

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$registerDebugTypes", `List([`List(items)])) =>
      let types =
        items
        |> List.filter_map(
             fun
             | `String(str) => Some(str)
             | _ => None,
           );
      Ok(RegisterDebugTypes(types));
    | _ => Error("Unhandled method: " ++ method)
    };
  };
};

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

module ExtensionService = {
  [@deriving show]
  type msg =
    | ActivateExtension({
        extensionId: string,
        activationEvent: option(string),
      })
    | WillActivateExtension({extensionId: string})
    | DidActivateExtension({
        extensionId: string,
        //startup: bool,
        codeLoadingTime: int,
        activateCallTime: int,
        activateResolvedTime: int,
      })
    //activationEvent: option(string),
    | ExtensionActivationError({
        extensionId: string,
        errorMessage: string,
      })
    | ExtensionRuntimeError({extensionId: string});
  // TODO: Error?

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$activateExtension", `List([`String(extensionId)])) =>
      Ok(ActivateExtension({extensionId, activationEvent: None}))
    | (
        "$activateExtension",
        `List([`String(extensionId), activationEventJson]),
      ) =>
      let activationEvent =
        switch (activationEventJson) {
        | `String(v) => Some(v)
        | _ => None
        };

      Ok(ActivateExtension({extensionId, activationEvent}));
    | (
        "$onExtensionActivationError",
        `List([`String(extensionId), `String(errorMessage)]),
      ) =>
      Ok(ExtensionActivationError({extensionId, errorMessage}))
    | ("$onWillActivateExtension", `List([`String(extensionId)])) =>
      Ok(WillActivateExtension({extensionId: extensionId}))
    | (
        "$onDidActivateExtension",
        `List([
          `String(extensionId),
          `Int(codeLoadingTime),
          `Int(activateCallTime),
          `Int(activateResolvedTime),
          ..._args,
        ]),
      ) =>
      Ok(
        DidActivateExtension({
          extensionId,
          codeLoadingTime,
          activateCallTime,
          activateResolvedTime,
        }),
      )
    | ("$onExtensionRuntimeError", `List([`String(extensionId), ..._args])) =>
      Ok(ExtensionRuntimeError({extensionId: extensionId}))
    | _ => Error("Unhandled method: " ++ method)
    };
  };
};

module LanguageFeatures = {
  [@deriving show]
  type msg =
      | RegisterDocumentHighlightProvider({
          handle: int,
          selector: list(DocumentFilter.t),
      })
    | RegisterDefinitionSupport({
        handle: int,
        selector: list(DocumentFilter.t),
      })
    | RegisterDeclarationSupport({
        handle: int,
        selector: list(DocumentFilter.t),
      })
    | RegisterImplementationSupport({
        handle: int,
        selector: list(DocumentFilter.t),
      })
    | RegisterTypeDefinitionSupport({
        handle: int,
        selector: list(DocumentFilter.t),
      })
    | RegisterSuggestSupport({
        handle: int,
        selector: list(DocumentFilter.t),
        triggerCharacters: list(string),
        supportsResolveDetails: bool,
        extensionId: string,
      })
    | Unregister({handle: int});

  let parseDocumentSelector = json => {
    Json.Decode.(json |> decode_value(list(DocumentFilter.decode)));
  };

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$unregister", `List([`Int(handle)])) =>
      Ok(Unregister({handle: handle}))

    | ("$registerDefinitionSupport", `List([`Int(handle), selectorJson])) =>
      selectorJson
      |> parseDocumentSelector
      |> Result.map(selector => {
           RegisterDefinitionSupport({handle, selector})
         })
      |> Result.map_error(Json.Decode.string_of_error)
    | ("$registerDeclarationSupport", `List([`Int(handle), selectorJson])) =>
      selectorJson
      |> parseDocumentSelector
      |> Result.map(selector => {
           RegisterDeclarationSupport({handle, selector})
         })
      |> Result.map_error(Json.Decode.string_of_error)
    | (
        "$registerImplementationSupport",
        `List([`Int(handle), selectorJson]),
      ) =>
      selectorJson
      |> parseDocumentSelector
      |> Result.map(selector => {
           RegisterImplementationSupport({handle, selector})
         })
      |> Result.map_error(Json.Decode.string_of_error)
    | (
        "$registerTypeDefinitionSupport",
        `List([`Int(handle), selectorJson]),
      ) =>
      switch (parseDocumentSelector(selectorJson)) {
      | Ok(selector) =>
        Ok(RegisterImplementationSupport({handle, selector}))
      | Error(error) => Error(Json.Decode.string_of_error(error))
      }
    | (
        "$registerSuggestSupport",
        `List([
          `Int(handle),
          selectorJson,
          triggerCharactersJson,
          `Bool(supportsResolveDetails),
          `String(extensionId),
        ]),
      ) =>
      open Json.Decode;

      let ret = {
        open Base.Result.Let_syntax;
        let%bind selector =
          selectorJson |> decode_value(list(DocumentFilter.decode));

        let%bind triggerCharacters =
          triggerCharactersJson
          |> decode_value(list(list(string)))
          |> Result.map(List.flatten);

        Ok(
          RegisterSuggestSupport({
            handle,
            selector,
            triggerCharacters,
            supportsResolveDetails,
            extensionId,
          }),
        );
      };

      ret |> Result.map_error(string_of_error);

    | _ => Error("Unhandled method: " ++ method)
    };
  };
};

module MessageService = {
  [@deriving show]
  type severity =
    | Ignore
    | Info
    | Warning
    | Error;

  let intToSeverity =
    fun
    | 0 => Ignore
    | 1 => Info
    | 2 => Warning
    | 3 => Error
    | _ => Ignore;

  [@deriving show]
  type msg =
    | ShowMessage({
        severity,
        message: string,
        extensionId: option(string),
      });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | (
        "$showMessage",
        `List([`Int(severity), `String(message), options, ..._]),
      ) =>
      let extensionId =
        Yojson.Safe.Util.(
          options
          |> member("extension")
          |> member("identifier")
          |> to_string_option
        );
      Ok(
        ShowMessage({
          severity: intToSeverity(severity),
          message,
          extensionId,
        }),
      );
    | _ =>
      Error(
        "Unable to parse method: "
        ++ method
        ++ " with args: "
        ++ Yojson.Safe.to_string(args),
      )
    };
  };
};
module StatusBar = {
  [@deriving show]
  type alignment =
    | Left
    | Right;

  let stringToAlignment =
    fun
    | "0" => Left
    | "1" => Right
    | _ => Left;

  [@deriving show]
  type msg =
    | SetEntry({
        id: string,
        text: string,
        source: string,
        alignment,
        priority: int,
      });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | (
        "$setEntry",
        `List([
          `String(id),
          _,
          `String(source),
          `String(text),
          _,
          _,
          _,
          `String(alignment),
          `String(priority),
        ]),
      ) =>
      let alignment = stringToAlignment(alignment);
      let priority = int_of_string_opt(priority) |> Option.value(~default=0);
      Ok(SetEntry({id, source, text, alignment, priority}));
    | _ =>
      Error(
        "Unable to parse method: "
        ++ method
        ++ " with args: "
        ++ Yojson.Safe.to_string(args),
      )
    };
  };
};
module Telemetry = {
  [@deriving show]
  type msg =
    | PublicLog({
        eventName: string,
        data: Yojson.Safe.t,
      })
    | PublicLog2({
        eventName: string,
        data: Yojson.Safe.t,
      });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$publicLog", `List([`String(eventName), data, ..._])) =>
      Ok(PublicLog({eventName, data}))
    | ("$publicLog2", `List([`String(eventName), data, ..._])) =>
      Ok(PublicLog2({eventName, data}))
    | _ => Error("Unhandled method: " ++ method)
    };
  };
};
module TerminalService = {
  [@deriving show]
  type msg =
    | SendProcessTitle({
        terminalId: int,
        title: string,
      })
    | SendProcessData({
        terminalId: int,
        data: string,
      })
    | SendProcessPid({
        terminalId: int,
        pid: int,
      })
    | SendProcessExit({
        terminalId: int,
        exitCode: int,
      });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method) {
    | "$sendProcessTitle" =>
      switch (args) {
      | `List([`Int(terminalId), `String(title)]) =>
        Ok(SendProcessTitle({terminalId, title}))
      | _ => Error("Unexpected arguments for $sendProcessTitle")
      }

    | "$sendProcessData" =>
      switch (args) {
      | `List([`Int(terminalId), `String(data)]) =>
        Ok(SendProcessData({terminalId, data}))
      | _ => Error("Unexpected arguments for $sendProcessData")
      }

    | "$sendProcessPid" =>
      switch (args) {
      | `List([`Int(terminalId), `Int(pid)]) =>
        Ok(SendProcessPid({terminalId, pid}))

      | _ => Error("Unexpected arguments for $sendProcessPid")
      }

    | "$sendProcessExit" =>
      switch (args) {
      | `List([`Int(terminalId), `Int(exitCode)]) =>
        Ok(SendProcessExit({terminalId, exitCode}))

      | _ => Error("Unexpected arguments for $sendProcessExit")
      }

    | _ =>
      Error(
        Printf.sprintf(
          "Unhandled Terminal message - %s: %s",
          method,
          Yojson.Safe.to_string(args),
        ),
      )
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
  | LanguageFeatures(LanguageFeatures.msg)
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
