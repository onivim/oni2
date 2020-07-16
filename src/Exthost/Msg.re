module ExtCommand = Command;
open Oni_Core;

module Internal = {
  let decode_value = (decoder, json) => {
    json
    |> Json.Decode.decode_value(decoder)
    |> Result.map_error(Json.Decode.string_of_error);
  };
};

module Decode = {
  open Json.Decode;

  let int =
    one_of([
      ("int", int),
      (
        "string",
        string
        |> map(int_of_string_opt)
        |> and_then(
             fun
             | Some(num) => succeed(num)
             | None => fail("Unable to parse number"),
           ),
      ),
    ]);

  let id =
    one_of([("string", string), ("int", int |> map(string_of_int))]);
};

module Clipboard = {
  [@deriving show]
  type msg =
    | ReadText
    | WriteText(string);

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$readText", _) => Ok(ReadText)
    | ("$writeText", `List([`String(text)])) => Ok(WriteText(text))
    | _ => Error("Unhandled clipboard method: " ++ method)
    };
  };
};

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

  module Decode = {
    open Json.Decode;

    let emptyDiagnostics = null |> map(_ => []);
    let diagnostics =
      one_of([
        ("list", list(Diagnostic.decode)),
        ("empty", emptyDiagnostics),
      ]);

    let entry =
      Pipeline.(
        decode((uri, diagnostics) => (uri, diagnostics))
        |> custom(index(0, Uri.decode))
        |> custom(index(1, diagnostics))
      );
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
    | ("$changeMany", `List([`String(owner), diagnosticsJson])) =>
      diagnosticsJson
      |> Json.Decode.decode_value(Json.Decode.list(Decode.entry))
      |> Result.map(entries => ChangeMany({owner, entries}))
      |> Result.map_error(Json.Decode.string_of_error)
    | ("$clear", `List([`String(owner)])) => Ok(Clear({owner: owner}))
    | _ => Error("Unhandled method: " ++ method)
    };
  };

  let%test "null list (regression test for #1839)" = {
    let json =
      {|
      ["typescript",[[{"$mid":1,"fsPath":"/Users/onivim/bootstrap.js","external":"file:///Users/onivim/scripts/bootstrap.js","path":"/Users/onivim/bootstrap.js","scheme":"file"},null]]]
    |}
      |> Yojson.Safe.from_string;

    let parsedResult = handle("$changeMany", json);
    parsedResult
    == Ok(
         ChangeMany({
           owner: "typescript",
           entries: [(Uri.fromPath("/Users/onivim/bootstrap.js"), [])],
         }),
       );
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
        extensionId: ExtensionId.t,
        activationEvent: option(string),
      })
    | WillActivateExtension({extensionId: ExtensionId.t})
    | DidActivateExtension({
        extensionId: ExtensionId.t,
        //startup: bool,
        codeLoadingTime: int,
        activateCallTime: int,
        activateResolvedTime: int,
      })
    //activationEvent: option(string),
    | ExtensionActivationError({
        extensionId: ExtensionId.t,
        errorMessage: string,
      })
    | ExtensionRuntimeError({extensionId: ExtensionId.t});
  let withExtensionId = (f, extensionIdJson) => {
    extensionIdJson
    |> Json.Decode.decode_value(ExtensionId.decode)
    |> Result.map(f)
    |> Result.map_error(Json.Decode.string_of_error);
  };

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$activateExtension", `List([extensionIdJson])) =>
      extensionIdJson
      |> withExtensionId(extensionId => {
           ActivateExtension({extensionId, activationEvent: None})
         })
    | ("$activateExtension", `List([extensionIdJson, activationEventJson])) =>
      let activationEvent =
        switch (activationEventJson) {
        | `String(v) => Some(v)
        | _ => None
        };

      extensionIdJson
      |> withExtensionId(extensionId => {
           ActivateExtension({extensionId, activationEvent})
         });
    | (
        "$onExtensionActivationError",
        `List([extensionIdJson, `String(errorMessage)]),
      ) =>
      extensionIdJson
      |> withExtensionId(extensionId => {
           ExtensionActivationError({extensionId, errorMessage})
         })
    | ("$onWillActivateExtension", `List([extensionIdJson])) =>
      extensionIdJson
      |> withExtensionId(extensionId => {
           WillActivateExtension({extensionId: extensionId})
         })
    | (
        "$onDidActivateExtension",
        `List([
          extensionIdJson,
          `Int(codeLoadingTime),
          `Int(activateCallTime),
          `Int(activateResolvedTime),
          ..._args,
        ]),
      ) =>
      extensionIdJson
      |> withExtensionId(extensionId => {
           DidActivateExtension({
             extensionId,
             codeLoadingTime,
             activateCallTime,
             activateResolvedTime,
           })
         })
    | ("$onExtensionRuntimeError", `List([extensionIdJson, ..._args])) =>
      extensionIdJson
      |> withExtensionId(extensionId => {
           ExtensionRuntimeError({extensionId: extensionId})
         })
    | _ => Error("Unhandled method: " ++ method)
    };
  };
};

module FileSystem = {
  open Files;

  [@deriving show]
  type msg =
    | RegisterFileSystemProvider({
        handle: int,
        scheme: string,
        capabilities: FileSystemProviderCapabilities.t,
      })
    | UnregisterProvider({handle: int})
    | OnFileSystemChange({
        handle: int,
        resource: list(FileChange.t),
      })
    | Stat({uri: Uri.t})
    | ReadDir({uri: Uri.t})
    | ReadFile({uri: Uri.t})
    | WriteFile({
        uri: Uri.t,
        bytes: Bytes.t,
      })
    | Rename({
        source: Uri.t,
        target: Uri.t,
        opts: FileOverwriteOptions.t,
      })
    | Copy({
        source: Uri.t,
        target: Uri.t,
        opts: FileOverwriteOptions.t,
      })
    | Mkdir({uri: Uri.t})
    | Delete({
        uri: Uri.t,
        opts: FileDeleteOptions.t,
      });

  let handle = (method, args: Yojson.Safe.t) => {
    Base.Result.Let_syntax.(
      switch (method, args) {
      | ("$stat", `List([uriJson])) =>
        let%bind uri = uriJson |> Internal.decode_value(Uri.decode);
        Ok(Stat({uri: uri}));
      | ("$readdir", `List([uriJson])) =>
        let%bind uri = uriJson |> Internal.decode_value(Uri.decode);
        Ok(ReadDir({uri: uri}));
      | ("$readFile", `List([uriJson])) =>
        let%bind uri = uriJson |> Internal.decode_value(Uri.decode);
        Ok(ReadFile({uri: uri}));
      | ("$writeFile", `List([uriJson, `String(buffer)])) =>
        let%bind uri = uriJson |> Internal.decode_value(Uri.decode);
        let bytes = Bytes.of_string(buffer);
        Ok(WriteFile({uri, bytes}));
      | ("$rename", `List([sourceJson, targetJson, optsJson])) =>
        let%bind source = sourceJson |> Internal.decode_value(Uri.decode);
        let%bind target = targetJson |> Internal.decode_value(Uri.decode);
        let%bind opts =
          optsJson |> Internal.decode_value(FileOverwriteOptions.decode);
        Ok(Rename({source, target, opts}));
      | ("$copy", `List([sourceJson, targetJson, optsJson])) =>
        let%bind source = sourceJson |> Internal.decode_value(Uri.decode);
        let%bind target = targetJson |> Internal.decode_value(Uri.decode);
        let%bind opts =
          optsJson |> Internal.decode_value(FileOverwriteOptions.decode);
        Ok(Copy({source, target, opts}));
      | ("$mkdir", `List([uriJson])) =>
        let%bind uri = uriJson |> Internal.decode_value(Uri.decode);
        Ok(Mkdir({uri: uri}));
      | ("$delete", `List([uriJson, deleteOptsJson])) =>
        let%bind uri = uriJson |> Internal.decode_value(Uri.decode);
        let%bind opts =
          deleteOptsJson |> Internal.decode_value(FileDeleteOptions.decode);
        Ok(Delete({uri, opts}));
      | _ => Error("Unhandled FileSystem method: " ++ method)
      }
    );
  };
};

module LanguageFeatures = {
  [@deriving show]
  type msg =
    | EmitCodeLensEvent({
        eventHandle: int,
        event: Yojson.Safe.t,
      }) // ??
    | RegisterCodeLensSupport({
        handle: int,
        selector: DocumentSelector.t,
        eventHandle: option(int),
      })
    | RegisterDocumentHighlightProvider({
        handle: int,
        selector: list(DocumentFilter.t),
      })
    | RegisterDocumentSymbolProvider({
        handle: int,
        selector: list(DocumentFilter.t),
        label: string,
      })
    | RegisterDefinitionSupport({
        handle: int,
        selector: list(DocumentFilter.t),
      })
    | RegisterDeclarationSupport({
        handle: int,
        selector: list(DocumentFilter.t),
      })
    | RegisterHoverProvider({
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
    | RegisterSignatureHelpProvider({
        handle: int,
        selector: DocumentSelector.t,
        metadata: SignatureHelp.ProviderMetadata.t,
      })
    | RegisterSuggestSupport({
        handle: int,
        selector: list(DocumentFilter.t),
        triggerCharacters: list(string),
        supportsResolveDetails: bool,
        extensionId: string,
      })
    | RegisterReferenceSupport({
        handle: int,
        selector: list(DocumentFilter.t),
      })
    | RegisterDocumentFormattingSupport({
        handle: int,
        selector: DocumentSelector.t,
        extensionId: ExtensionId.t,
        displayName: string,
      })
    | RegisterRangeFormattingSupport({
        handle: int,
        selector: DocumentSelector.t,
        extensionId: ExtensionId.t,
        displayName: string,
      })
    | RegisterOnTypeFormattingSupport({
        handle: int,
        selector: DocumentSelector.t,
        autoFormatTriggerCharacters: list(string),
        extensionId: ExtensionId.t,
      })
    | Unregister({handle: int});

  let parseDocumentSelector = json => {
    open Oni_Core.Utility;
    open Json.Decode;

    let decoder = list(DocumentFilter.decode);

    // There must be a cleaner way to handle this...
    let stringDecoder =
      string
      |> and_then(str => {
           let result =
             str
             |> JsonEx.from_string
             |> ResultEx.flatMap(v =>
                  v
                  |> decode_value(decoder)
                  |> Result.map_error(Json.Decode.string_of_error)
                );
           switch (result) {
           | Ok(v) => succeed(v)
           | Error(m) => fail(m)
           };
         });

    let composite = one_of([("json", decoder), ("string", stringDecoder)]);

    json |> decode_value(composite);
  };

  let decodeStringOrInt = {
    open Json.Decode;
    let stringToInt =
      string
      |> map(int_of_string_opt)
      |> and_then(
           fun
           | Some(i) => succeed(i)
           | None => fail("Unable to parse int"),
         );

    one_of([("int", int), ("string", stringToInt)]);
  };

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$unregister", `List([`Int(handle)])) =>
      Ok(Unregister({handle: handle}))
    | (
        "$registerDocumentHighlightProvider",
        `List([`Int(handle), selectorJson]),
      ) =>
      switch (parseDocumentSelector(selectorJson)) {
      | Ok(selector) =>
        Ok(RegisterDocumentHighlightProvider({handle, selector}))
      | Error(error) => Error(Json.Decode.string_of_error(error))
      }
    | ("$emitCodeLensEvent", `List([`Int(eventHandle), json])) =>
      Ok(EmitCodeLensEvent({eventHandle, event: json}))
    | (
        "$registerCodeLensSupport",
        `List([handleJson, selectorJson, eventHandleJson]),
      ) =>
      let ret = {
        open Base.Result.Let_syntax;
        open Json.Decode;
        let%bind handle = handleJson |> decode_value(decodeStringOrInt);
        let%bind selector = parseDocumentSelector(selectorJson);

        let%bind eventHandle =
          eventHandleJson |> decode_value(nullable(int));

        Ok(RegisterCodeLensSupport({handle, selector, eventHandle}));
      };
      ret |> Result.map_error(Json.Decode.string_of_error);
    | (
        "$registerDocumentSymbolProvider",
        `List([`Int(handle), selectorJson, `String(label)]),
      ) =>
      switch (parseDocumentSelector(selectorJson)) {
      | Ok(selector) =>
        Ok(RegisterDocumentSymbolProvider({handle, selector, label}))
      | Error(error) => Error(Json.Decode.string_of_error(error))
      }
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
    | ("$registerHoverProvider", `List([`Int(handle), selectorJson])) =>
      selectorJson
      |> parseDocumentSelector
      |> Result.map(selector => {RegisterHoverProvider({handle, selector})})
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

    | ("$registerReferenceSupport", `List([`Int(handle), selectorJson])) =>
      switch (parseDocumentSelector(selectorJson)) {
      | Ok(selector) => Ok(RegisterReferenceSupport({handle, selector}))
      | Error(error) => Error(Json.Decode.string_of_error(error))
      }

    | (
        "$registerSignatureHelpProvider",
        `List([`Int(handle), selectorJson, metadataJson]),
      ) =>
      open Json.Decode;

      let ret = {
        open Base.Result.Let_syntax;
        let%bind selector =
          selectorJson |> decode_value(list(DocumentFilter.decode));

        let%bind metadata =
          metadataJson |> decode_value(SignatureHelp.ProviderMetadata.decode);

        Ok(RegisterSignatureHelpProvider({handle, selector, metadata}));
      };

      ret |> Result.map_error(string_of_error);
    | (
        "$registerSuggestSupport",
        `List([
          `Int(handle),
          selectorJson,
          triggerCharactersJson,
          `Bool(supportsResolveDetails),
          extensionIdJson,
        ]),
      ) =>
      open Json.Decode;
      let nestedListDecoder = list(list(string)) |> map(List.flatten);

      let decodeTriggerCharacters =
        one_of([
          ("nestedList", nestedListDecoder),
          ("stringList", list(string)),
        ]);

      let ret = {
        open Base.Result.Let_syntax;
        let%bind selector =
          selectorJson |> decode_value(list(DocumentFilter.decode));

        let%bind triggerCharacters =
          triggerCharactersJson |> decode_value(decodeTriggerCharacters);

        let%bind extensionId =
          extensionIdJson |> decode_value(ExtensionId.decode);

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
    | (
        "$registerDocumentFormattingSupport",
        `List([
          `Int(handle),
          selectorJson,
          extensionIdJson,
          displayNameJson,
        ]),
      ) =>
      open Json.Decode;

      let ret = {
        open Base.Result.Let_syntax;
        let%bind selector =
          selectorJson |> decode_value(list(DocumentFilter.decode));

        let%bind extensionId =
          extensionIdJson |> decode_value(ExtensionId.decode);

        let%bind displayName = displayNameJson |> decode_value(string);

        Ok(
          RegisterDocumentFormattingSupport({
            handle,
            selector,
            extensionId,
            displayName,
          }),
        );
      };
      ret |> Result.map_error(string_of_error);
    | (
        "$registerRangeFormattingSupport",
        `List([
          `Int(handle),
          selectorJson,
          extensionIdJson,
          displayNameJson,
        ]),
      ) =>
      open Json.Decode;

      let ret = {
        open Base.Result.Let_syntax;
        let%bind selector =
          selectorJson |> decode_value(list(DocumentFilter.decode));

        let%bind extensionId =
          extensionIdJson |> decode_value(ExtensionId.decode);

        let%bind displayName = displayNameJson |> decode_value(string);

        Ok(
          RegisterRangeFormattingSupport({
            handle,
            selector,
            extensionId,
            displayName,
          }),
        );
      };
      ret |> Result.map_error(string_of_error);
    | (
        "$registerOnTypeFormattingSupport",
        `List([
          `Int(handle),
          selectorJson,
          triggerCharacterJson,
          extensionIdJson,
        ]),
      ) =>
      open Json.Decode;

      let ret = {
        open Base.Result.Let_syntax;
        let%bind selector =
          selectorJson |> decode_value(list(DocumentFilter.decode));

        let%bind triggerCharacters =
          triggerCharacterJson |> decode_value(list(string));
        let%bind extensionId =
          extensionIdJson |> decode_value(ExtensionId.decode);

        Ok(
          RegisterOnTypeFormattingSupport({
            handle,
            selector,
            autoFormatTriggerCharacters: triggerCharacters,
            extensionId,
          }),
        );
      };
      ret |> Result.map_error(string_of_error);
    | _ =>
      Error(
        Printf.sprintf(
          "Unhandled method: %s - Args: %s",
          method,
          Yojson.Safe.to_string(args),
        ),
      )
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
        `List([`Int(severity), `String(message), _options, ..._]),
      ) =>
      try(
        Ok(
          ShowMessage({
            severity: intToSeverity(severity),
            message,
            // TODO:
            // Fix this up
            extensionId: None,
          }),
        )
      ) {
      | exn => Error(Printexc.to_string(exn))
      }
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

  let intToAlignment =
    fun
    | 0 => Left
    | 1 => Right
    | _ => Left;

  [@deriving show]
  type msg =
    | SetEntry({
        id: string,
        label: Label.t,
        source: string,
        alignment,
        command: option(ExtCommand.t),
        color: option(Color.t),
        priority: int,
      })
    | Dispose({id: int});

  let parseCommand = commandJson =>
    switch (commandJson) {
    | `String(jsonString) =>
      jsonString
      |> Yojson.Safe.from_string
      |> Json.Decode.decode_value(Json.Decode.nullable(ExtCommand.decode))
      |> Result.map_error(Json.Decode.string_of_error)
    | _ => Ok(None)
    };

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | (
        "$setEntry",
        `List([
          idJson,
          _,
          `String(source),
          labelJson,
          _tooltip,
          commandJson,
          colorJson,
          alignmentJson,
          priorityJson,
        ]),
      ) =>
      open Base.Result.Let_syntax;
      let%bind id = idJson |> Internal.decode_value(Decode.id);
      let%bind command = parseCommand(commandJson);
      let%bind color =
        colorJson
        |> Internal.decode_value(Json.Decode.nullable(Color.decode));
      let%bind label = labelJson |> Internal.decode_value(Label.decode);

      let%bind alignmentNumber =
        alignmentJson |> Internal.decode_value(Decode.int);
      let alignment = alignmentNumber |> intToAlignment;
      let%bind priority = priorityJson |> Internal.decode_value(Decode.int);
      Ok(SetEntry({id, source, label, alignment, color, priority, command}));
    | ("$dispose", `List([`Int(id)])) => Ok(Dispose({id: id}))
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

module SCM = {
  [@deriving show]
  type msg =
    | RegisterSourceControl({
        handle: int,
        id: string,
        label: string,
        rootUri: option(Uri.t),
      })
    | UnregisterSourceControl({handle: int})
    | UpdateSourceControl({
        handle: int,
        hasQuickDiffProvider: option(bool),
        count: option(int),
        commitTemplate: option(string),
        acceptInputCommand: option(SCM.command),
      })
    // statusBarCommands: option(_),
    | RegisterSCMResourceGroup({
        provider: int,
        handle: int,
        id: string,
        label: string,
      })
    | UnregisterSCMResourceGroup({
        provider: int,
        handle: int,
      })
    | SpliceSCMResourceStates({
        handle: int,
        splices: list(SCM.Resource.Splices.t),
      });
  //additions: list(SCM.Resource.t),

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method) {
    | "$registerSourceControl" =>
      switch (args) {
      | `List([`Int(handle), `String(id), `String(label), rootUri]) =>
        let rootUri = Uri.of_yojson(rootUri) |> Stdlib.Result.to_option;
        Ok(RegisterSourceControl({handle, id, label, rootUri}));
      | `List([`String(handleStr), `String(id), `String(label), rootUri]) =>
        let rootUri = Uri.of_yojson(rootUri) |> Stdlib.Result.to_option;
        let maybeHandle = int_of_string_opt(handleStr);
        switch (maybeHandle) {
        | Some(handle) =>
          Ok(RegisterSourceControl({handle, id, label, rootUri}))
        | None =>
          Error("Expected number for handle, but received: " ++ handleStr)
        };
      | _ => Error("Unexpected arguments for $registerSourceControl")
      }

    | "$unregisterSourceControl" =>
      switch (args) {
      | `List([`Int(handle)]) =>
        Ok(UnregisterSourceControl({handle: handle}))

      | _ => Error("Unexpected arguments for $unregisterSourceControl")
      }

    | "$updateSourceControl" =>
      switch (args) {
      | `List([`Int(handle), features]) =>
        Yojson.Safe.Util.(
          Ok(
            UpdateSourceControl({
              handle,
              hasQuickDiffProvider:
                features |> member("hasQuickDiffProvider") |> to_bool_option,
              count: features |> member("count") |> to_int_option,
              commitTemplate:
                features |> member("commitTemplate") |> to_string_option,
              acceptInputCommand:
                features |> member("acceptInputCommand") |> SCM.Decode.command,
            }),
          )
        )

      | _ => Error("Unexpected arguments for $updateSourceControl")
      }

    | "$registerGroup" =>
      switch (args) {
      | `List([`Int(provider), `Int(handle), `String(id), `String(label)]) =>
        Ok(RegisterSCMResourceGroup({provider, handle, id, label}))

      | _ => Error("Unexpected arguments for $registerGroup")
      }

    | "$unregisterGroup" =>
      switch (args) {
      | `List([`Int(handle), `Int(provider)]) =>
        Ok(UnregisterSCMResourceGroup({provider, handle}))

      | _ => Error("Unexpected arguments for $unregisterGroup")
      }

    | "$spliceResourceStates" =>
      switch (args) {
      | `List([`Int(handle), splicesJson]) =>
        let splicesResult =
          Json.Decode.(
            splicesJson
            |> Json.Decode.decode_value(list(SCM.Resource.Decode.splices))
          );

        switch (splicesResult) {
        | Ok(splices) => Ok(SpliceSCMResourceStates({handle, splices}))
        | Error(err) => Error(Json.Decode.string_of_error(err))
        };
      | _ => Error("Unexpected arguments for $spliceResourceStates")
      }
    | _ =>
      Error(
        Printf.sprintf(
          "Unhandled SCM message - %s: %s",
          method,
          Yojson.Safe.to_string(args),
        ),
      )
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
    | SendProcessReady({
        terminalId: int,
        pid: int,
        workingDirectory: string,
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

    | "$sendProcessReady" =>
      switch (args) {
      | `List([`Int(terminalId), `Int(pid), `String(workingDirectory)]) =>
        Ok(SendProcessReady({terminalId, pid, workingDirectory}))

      | _ => Error("Unexpected arguments for $sendProcessReady")
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
  | Clipboard(Clipboard.msg)
  | Commands(Commands.msg)
  | DebugService(DebugService.msg)
  | Decorations(Decorations.msg)
  | Diagnostics(Diagnostics.msg)
  | DocumentContentProvider(DocumentContentProvider.msg)
  | ExtensionService(ExtensionService.msg)
  | FileSystem(FileSystem.msg)
  | LanguageFeatures(LanguageFeatures.msg)
  | MessageService(MessageService.msg)
  | SCM(SCM.msg)
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
