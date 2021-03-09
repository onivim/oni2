module ExtCommand = Command;
module ExtConfig = Configuration;
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

  let bool = one_of([("bool", bool), ("false", succeed(false))]);

  let int =
    one_of([
      ("float", float |> map(int_of_float)),
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

module QuickOpen = {
  [@deriving show]
  type msg =
    // TODO: How to handle incoming cancellation token?
    | Show({
        instance: int,
        options: QuickOpen.Options.t,
      }) // Returns a promise / id
    | SetItems({
        instance: int,
        items: list(QuickOpen.Item.t),
      })
    | SetError({
        instance: int,
        error: Yojson.Safe.t,
      })
    | Input({
        options: InputBoxOptions.t,
        validateInput: bool,
      }) // Returns a string promise
    | CreateOrUpdate({params: QuickOpen.t})
    | Dispose({id: int});

  let handle = (method, args: Yojson.Safe.t) => {
    Base.Result.Let_syntax.(
      switch (method, args) {
      | ("$show", `List([`Int(instance), optionsJson])) =>
        let%bind maybeOptions =
          optionsJson
          |> Internal.decode_value(
               Json.Decode.nullable(QuickOpen.Options.decode),
             );
        let options =
          maybeOptions |> Option.value(~default=QuickOpen.Options.default);
        Ok(Show({instance, options}));
      | ("$setItems", `List([`Int(instance), itemsJson])) =>
        let%bind items =
          itemsJson
          |> Internal.decode_value(Json.Decode.list(QuickOpen.Item.decode));
        Ok(SetItems({instance, items}));
      | ("$setError", `List([`Int(instance), errorJson])) =>
        Ok(SetError({instance, error: errorJson}))
      | ("$input", `List([inputBoxOptionsJson, `Bool(validateInput)])) =>
        let%bind inputBox =
          inputBoxOptionsJson |> Internal.decode_value(InputBoxOptions.decode);
        Ok(Input({options: inputBox, validateInput}));
      | ("$createOrUpdate", `List([paramsJson])) =>
        let%bind params =
          paramsJson |> Internal.decode_value(QuickOpen.decode);
        Ok(CreateOrUpdate({params: params}));
      | ("$dispose", `List([`Int(id)])) => Ok(Dispose({id: id}))
      | _ =>
        Error(
          "Unhandled quickOpen method: "
          ++ method
          ++ " - args: "
          ++ Yojson.Safe.to_string(args),
        )
      }
    );
  };
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

module Configuration = {
  [@deriving show]
  type msg =
    | UpdateConfigurationOption({
        target: option(ExtConfig.Target.t),
        key: string,
        value: Yojson.Safe.t,
        overrides: option(ExtConfig.Overrides.t),
        scopeToLanguage: bool,
      })
    | RemoveConfigurationOption({
        target: option(ExtConfig.Target.t),
        key: string,
        overrides: option(ExtConfig.Overrides.t),
        scopeToLanguage: bool,
      });

  let handle = (method, args: Yojson.Safe.t) => {
    Base.Result.Let_syntax.(
      switch (method, args) {
      | (
          "$updateConfigurationOption",
          `List([
            targetJson,
            `String(key),
            valueJson,
            overridesJson,
            scopeToLanguageJson,
          ]),
        ) =>
        let%bind target =
          targetJson
          |> Internal.decode_value(
               Json.Decode.(nullable(ExtConfig.Target.decode)),
             );

        let%bind overrides =
          overridesJson
          |> Internal.decode_value(
               Json.Decode.(nullable(ExtConfig.Overrides.decode)),
             );

        let%bind scopeToLanguage =
          scopeToLanguageJson |> Internal.decode_value(Decode.bool);
        Ok(
          UpdateConfigurationOption({
            target,
            key,
            value: valueJson,
            overrides,
            scopeToLanguage,
          }),
        );

      | (
          "$removeConfigurationOption",
          `List([
            targetJson,
            `String(key),
            overridesJson,
            scopeToLanguageJson,
          ]),
        ) =>
        let%bind target =
          targetJson
          |> Internal.decode_value(
               Json.Decode.(nullable(ExtConfig.Target.decode)),
             );

        let%bind overrides =
          overridesJson
          |> Internal.decode_value(
               Json.Decode.(nullable(ExtConfig.Overrides.decode)),
             );

        let%bind scopeToLanguage =
          scopeToLanguageJson |> Internal.decode_value(Decode.bool);
        Ok(
          RemoveConfigurationOption({
            target,
            key,
            overrides,
            scopeToLanguage,
          }),
        );
      | _ => Error("Unhandled Configuration method: " ++ method)
      }
    );
  };
};

module Console = {
  [@deriving show]
  type msg =
    | LogExtensionHostMessage({
        logType: string,
        severity: string,
        arguments: Yojson.Safe.t,
      });

  type logMessage = {
    logType: string,
    severity: string,
    arguments: Yojson.Safe.t,
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          logType: field.required("type", string),
          severity: field.required("severity", string),
          arguments: field.withDefault("arguments", `Null, value),
        }
      )
    );

  let handle = (method, args) => {
    switch (method) {
    | "$logExtensionHostMessage" =>
      open Base.Result.Let_syntax;

      let%bind {logType, severity, arguments} =
        args |> Internal.decode_value(decode);
      Ok(LogExtensionHostMessage({logType, severity, arguments}));
    | _ => Error("Console - unhandled method: " ++ method)
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

    | ("$onDidChange", `List([`Int(handle), `Null])) =>
      Ok(DecorationsDidChange({handle, uris: []}))

    | _ =>
      Error(
        "Unhandled method: "
        ++ method
        ++ " json: "
        ++ Yojson.Safe.to_string(args),
      )
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

module Documents = {
  module CreateOptions = {
    type t = {
      language: option(string),
      content: option(string),
    };

    let decode =
      Json.Decode.(
        obj(({field, _}) =>
          {
            language: field.optional("language", string),
            content: field.optional("content", string),
          }
        )
      );
  };

  [@deriving show]
  type msg =
    | TryCreateDocument({
        language: option(string),
        content: option(string),
      })
    | TryOpenDocument({uri: Oni_Core.Uri.t})
    | TrySaveDocument({uri: Oni_Core.Uri.t});

  let handle = (method, args) => {
    Base.Result.Let_syntax.(
      switch (method, args) {
      | ("$tryCreateDocument", `List([])) =>
        Ok(TryCreateDocument({language: None, content: None}))

      | ("$tryCreateDocument", `List([optionsJson])) =>
        let%bind options =
          optionsJson |> Internal.decode_value(CreateOptions.decode);
        Ok(
          TryCreateDocument({
            language: options.language,
            content: options.content,
          }),
        );

      | ("$tryOpenDocument", `List([uriJson])) =>
        let%bind uri = uriJson |> Internal.decode_value(Oni_Core.Uri.decode);
        Ok(TryOpenDocument({uri: uri}));

      | ("$trySaveDocument", `List([uriJson])) =>
        let%bind uri = uriJson |> Internal.decode_value(Oni_Core.Uri.decode);
        Ok(TrySaveDocument({uri: uri}));

      | _ => Error("Unhandled method: " ++ method)
      }
    );
  };
};

module DownloadService = {
  [@deriving show]
  type msg =
    | Download({
        uri: Oni_Core.Uri.t,
        dest: Oni_Core.Uri.t,
      });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$download", `List([uriJson, toUriJson])) =>
      open Base.Result.Let_syntax;

      let%bind uri = uriJson |> Internal.decode_value(Oni_Core.Uri.decode);
      let%bind dest = toUriJson |> Internal.decode_value(Oni_Core.Uri.decode);

      Ok(Download({uri, dest}));
    | _ => Error("DownloadService - unhandled method: " ++ method)
    };
  };
};

module Errors = {
  [@deriving show]
  type msg =
    | OnUnexpectedError(Yojson.Safe.t);

  let handle = (method, args) => {
    switch (method, args) {
    | ("$onUnexpectedError", args) => Ok(OnUnexpectedError(args))
    | _ => Error("Errors - unhandled method: " ++ method)
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
    | ExtensionActivationError({
        extensionId: ExtensionId.t,
        error: ExtensionActivationError.t,
      })
    | ExtensionRuntimeError({
        extensionId: ExtensionId.t,
        errorsJson: list(Yojson.Safe.t),
      });

  let handle = (method, args: Yojson.Safe.t) => {
    Base.Result.Let_syntax.(
      {
        switch (method, args) {
        | ("$activateExtension", `List([extensionIdJson])) =>
          let%bind extensionId =
            extensionIdJson |> Internal.decode_value(ExtensionId.decode);
          Ok(ActivateExtension({extensionId, activationEvent: None}));

        | (
            "$activateExtension",
            `List([extensionIdJson, activationEventJson]),
          ) =>
          let activationEvent =
            switch (activationEventJson) {
            | `String(v) => Some(v)
            | _ => None
            };

          let%bind extensionId =
            extensionIdJson |> Internal.decode_value(ExtensionId.decode);
          Ok(ActivateExtension({extensionId, activationEvent}));
        | (
            "$onExtensionActivationError",
            `List([extensionIdJson, errorJson]),
          ) =>
          let%bind extensionId =
            extensionIdJson |> Internal.decode_value(ExtensionId.decode);

          let%bind error =
            errorJson
            |> Internal.decode_value(ExtensionActivationError.decode);

          Ok(ExtensionActivationError({extensionId, error}));

        | ("$onWillActivateExtension", `List([extensionIdJson])) =>
          let%bind extensionId =
            extensionIdJson |> Internal.decode_value(ExtensionId.decode);
          Ok(WillActivateExtension({extensionId: extensionId}));
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
          let%bind extensionId =
            extensionIdJson |> Internal.decode_value(ExtensionId.decode);
          Ok(
            DidActivateExtension({
              extensionId,
              codeLoadingTime,
              activateCallTime,
              activateResolvedTime,
            }),
          );
        | (
            "$onExtensionRuntimeError",
            `List([extensionIdJson, ...errorsJson]),
          ) =>
          let%bind extensionId =
            extensionIdJson |> Internal.decode_value(ExtensionId.decode);
          Ok(ExtensionRuntimeError({extensionId, errorsJson}));

        | _ => Error("Unhandled method: " ++ method)
        };
      }
    );
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

      | (
          "$registerFileSystemProvider",
          `List([`Int(handle), `String(scheme), capabilitiesJson]),
        ) =>
        let%bind capabilities =
          capabilitiesJson
          |> Internal.decode_value(FileSystemProviderCapabilities.decode);
        Ok(RegisterFileSystemProvider({handle, scheme, capabilities}));

      | ("$unregisterFileSystemProvider", `List([`Int(handle)])) =>
        Ok(UnregisterProvider({handle: handle}))

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
      })
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
    | RegisterRenameSupport({
        handle: int,
        selector: DocumentSelector.t,
        supportsResolveInitialValues: bool,
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

    | ("$emitCodeLensEvent", `List([`Int(eventHandle)])) =>
      Ok(EmitCodeLensEvent({eventHandle, event: `Null}))

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
        "$registerRenameSupport",
        `List([
          `Int(handle),
          selectorJson,
          `Bool(supportsResolveInitialValues),
        ]),
      ) =>
      open Json.Decode;
      open Base.Result.Let_syntax;
      let%bind selector =
        selectorJson |> Internal.decode_value(list(DocumentFilter.decode));

      Ok(
        RegisterRenameSupport({
          handle,
          selector,
          supportsResolveInitialValues,
        }),
      );

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

module Languages = {
  [@deriving show]
  type msg =
    | GetLanguages
    | ChangeLanguage({
        uri: Oni_Core.Uri.t,
        languageId: string,
      });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$getLanguages", _) => Ok(GetLanguages)

    | ("$changeLanguage", `List([uriJson, `String(languageId)])) =>
      open Base.Result.Let_syntax;

      let%bind uri = uriJson |> Internal.decode_value(Oni_Core.Uri.decode);

      Ok(ChangeLanguage({uri, languageId}));

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

module MessageService = {
  [@deriving show]
  type msg =
    | ShowMessage({
        severity: Message.severity,
        message: string,
        extensionId: option(string),
        commands: list(Message.Command.t),
      });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | (
        "$showMessage",
        `List([`Int(severity), `String(message), _options, commandsJson]),
      ) =>
      open Base.Result.Let_syntax;
      open Json.Decode;

      let%bind commands =
        commandsJson |> Internal.decode_value(list(Message.Command.decode));

      Ok(
        ShowMessage({
          severity: Message.intToSeverity(severity),
          message,
          // TODO:
          // Fix this up
          commands,
          extensionId: None,
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

module OutputService = {
  [@deriving show]
  type msg =
    | Register({
        label: string,
        log: bool,
        file: option(Oni_Core.Uri.t),
      })
    | Append({
        channelId: string,
        value: string,
      })
    | Update({channelId: string})
    | Clear({
        channelId: string,
        till: int,
      })
    | Reveal({
        channelId: string,
        preserveFocus: bool,
      })
    | Close({channelId: string})
    | Dispose({channelId: string});

  let handle = (method, args: Yojson.Safe.t) => {
    Base.Result.Let_syntax.(
      Json.Decode.(
        switch (method, args) {
        | ("$register", `List([`String(label), `Bool(log), maybeUriJson])) =>
          let%bind maybeUri =
            maybeUriJson
            |> Internal.decode_value(nullable(Oni_Core.Uri.decode));
          Ok(Register({label, log, file: maybeUri}));
        | ("$append", `List([`String(channelId), `String(value)])) =>
          Ok(Append({channelId, value}))
        | ("$update", `List([`String(channelId)])) =>
          Ok(Update({channelId: channelId}))
        | ("$clear", `List([`String(channelId), `Int(till)])) =>
          Ok(Clear({channelId, till}))
        | ("$reveal", `List([`String(channelId), `Bool(preserveFocus)])) =>
          Ok(Reveal({channelId, preserveFocus}))
        | ("$close", `List([`String(channelId)])) =>
          Ok(Close({channelId: channelId}))
        | ("$dispose", `List([`String(channelId)])) =>
          Ok(Dispose({channelId: channelId}))
        | _ => Error("Unable to parse OutputService method: " ++ method)
        }
      )
    );
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
        backgroundColor: option(Color.t),
        tooltip: option(string),
        priority: int,
      })
    | Dispose({id: int});

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | (
        "$setEntry",
        `List([
          idJson,
          _,
          `String(source),
          labelJson,
          tooltipJson,
          commandJson,
          colorJson,
          backgroundColorJson,
          alignmentJson,
          priorityJson,
          _accessibilityInfoJson,
        ]),
      ) =>
      open Base.Result.Let_syntax;
      open Json.Decode;

      let%bind id = idJson |> Internal.decode_value(Decode.id);
      let%bind command =
        commandJson |> Internal.decode_value(nullable(ExtCommand.decode));
      let%bind color =
        colorJson |> Internal.decode_value(nullable(Color.decode));
      let%bind backgroundColor =
        backgroundColorJson |> Internal.decode_value(nullable(Color.decode));
      let%bind tooltip =
        tooltipJson |> Internal.decode_value(nullable(string));
      let%bind label = labelJson |> Internal.decode_value(Label.decode);

      let%bind alignmentNumber =
        alignmentJson |> Internal.decode_value(Decode.int);
      let alignment = alignmentNumber |> intToAlignment;
      let%bind priority =
        priorityJson
        |> Internal.decode_value(nullable(Decode.int))
        |> Result.map(Option.value(~default=0));
      Ok(
        SetEntry({
          id,
          source,
          label,
          alignment,
          color,
          backgroundColor,
          priority,
          tooltip,
          command,
        }),
      );

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

module Storage = {
  [@deriving show]
  type msg =
    | GetValue({
        shared: bool,
        key: string,
      })
    | SetValue({
        shared: bool,
        key: string,
        value: Yojson.Safe.t,
      });

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method, args) {
    | ("$getValue", `List([`Bool(shared), `String(key)])) =>
      Ok(GetValue({shared, key}))
    | ("$setValue", `List([`Bool(shared), `String(key), valueJson])) =>
      Ok(SetValue({shared, key, value: valueJson}))
    | _ => Error("Unhandled storage method: " ++ method)
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

module Progress = {
  [@deriving show]
  type msg =
    | StartProgress({
        handle: int,
        options: Progress.Options.t,
      })
    | ProgressReport({
        handle: int,
        message: Progress.Step.t,
      })
    | ProgressEnd({handle: int});

  let handle = (method, args: Yojson.Safe.t) => {
    Base.Result.Let_syntax.(
      switch (method, args) {
      | ("$startProgress", `List([`Int(handle), optionsJson, ..._])) =>
        let%bind options =
          optionsJson |> Internal.decode_value(Progress.Options.decode);

        Ok(StartProgress({handle, options}));

      | ("$progressReport", `List([`Int(handle), messageJson])) =>
        let%bind message =
          messageJson |> Internal.decode_value(Progress.Step.decode);
        Ok(ProgressReport({handle, message}));

      | ("$progressEnd", `List([`Int(handle)])) =>
        Ok(ProgressEnd({handle: handle}))
      | _ => Error("Progress - unhandled method: " ++ method)
      }
    );
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
        features: SCM.ProviderFeatures.t,
      })
    // statusBarCommands: option(_),
    | RegisterSCMResourceGroups({
        provider: int,
        groups: list(SCM.Group.t),
        splices: list(SCM.Resource.Splices.t),
      })
    | UnregisterSCMResourceGroup({
        provider: int,
        handle: int,
      })
    | UpdateGroup({
        provider: int,
        handle: int,
        features: SCM.GroupFeatures.t,
      })
    | UpdateGroupLabel({
        provider: int,
        handle: int,
        label: string,
      })
    | SetInputBoxPlaceholder({
        handle: int,
        value: string,
      })
    | SetInputBoxVisibility({
        handle: int,
        visible: bool,
      })
    | SetValidationProviderIsEnabled({
        handle: int,
        enabled: bool,
      })
    | SpliceSCMResourceStates({
        handle: int,
        splices: list(SCM.Resource.Splices.t),
      });
  //additions: list(SCM.Resource.t),

  let handle = (method, args: Yojson.Safe.t) => {
    Base.Result.Let_syntax.(
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
        | `List([`Int(handle), featuresJson]) =>
          let%bind features =
            featuresJson |> Internal.decode_value(SCM.ProviderFeatures.decode);

          Ok(UpdateSourceControl({handle, features}));

        | _ => Error("Unexpected arguments for $updateSourceControl")
        }

      | "$registerGroups" =>
        switch (args) {
        | `List([`Int(provider), groupsJson, splicesJson]) =>
          open Json.Decode;
          let%bind groups =
            groupsJson |> Internal.decode_value(list(SCM.Group.decode));
          let%bind splices =
            splicesJson
            |> Internal.decode_value(list(SCM.Resource.Decode.splices));
          Ok(RegisterSCMResourceGroups({provider, groups, splices}));

        | _ => Error("Unexpected arguments for $registerGroup")
        }

      | "$updateGroup" =>
        switch (args) {
        | `List([`Int(provider), `Int(handle), featuresJson]) =>
          let%bind features =
            featuresJson |> Internal.decode_value(SCM.GroupFeatures.decode);
          Ok(UpdateGroup({provider, handle, features}));
        | _ => Error("Unexpected arguments for $updateGroup")
        }

      | "$updateGroupLabel" =>
        switch (args) {
        | `List([`Int(provider), `Int(handle), `String(label)]) =>
          Ok(UpdateGroupLabel({provider, handle, label}))
        | _ => Error("Unexpected arguments for $updateGroup")
        }

      | "$unregisterGroup" =>
        switch (args) {
        | `List([`Int(handle), `Int(provider)]) =>
          Ok(UnregisterSCMResourceGroup({provider, handle}))

        | _ => Error("Unexpected arguments for $unregisterGroup")
        }

      | "$setInputBoxPlaceholder" =>
        switch (args) {
        | `List([`Int(handle), `String(value)]) =>
          Ok(SetInputBoxPlaceholder({handle, value}))

        | _ => Error("Unexpected arguments for $setInputBoxPlaceholder")
        }

      | "$setInputBoxVisibility" =>
        switch (args) {
        | `List([`Int(handle), `Bool(visible)]) =>
          Ok(SetInputBoxVisibility({handle, visible}))

        | _ => Error("Unexpected arguments for $setInputBoxVisibility")
        }

      | "$setValidationProviderIsEnabled" =>
        switch (args) {
        | `List([`Int(handle), `Bool(enabled)]) =>
          Ok(SetValidationProviderIsEnabled({handle, enabled}))

        | _ =>
          Error("Unexpected arguments for $setValidationProviderIsEnabled")
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
      }
    );
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

module Window = {
  [@deriving show]
  type msg =
    | GetWindowVisibility
    | OpenUri({uri: Oni_Core.Uri.t});

  let handle = (method, args: Yojson.Safe.t) => {
    switch (method) {
    | "$getWindowVisibility" => Ok(GetWindowVisibility)

    | "$openUri" =>
      switch (args) {
      | `List([uriJson, _uriString, _options]) =>
        uriJson |> Uri.of_yojson |> Result.map(uri => {OpenUri({uri: uri})})
      | _ =>
        Error(
          "Unexpected arguments for $openUri: " ++ Yojson.Safe.to_string(args),
        )
      }

    | _ =>
      Error(
        Printf.sprintf(
          "Unhandled window message - %s: %s",
          method,
          Yojson.Safe.to_string(args),
        ),
      )
    };
  };
};

module Workspace = {
  [@deriving show]
  type msg =
    | SaveAll({includeUntitled: bool})
    | StartFileSearch({
        includePattern: option(string),
        //        includeFolder: option(Oni_Core.Uri.t),
        excludePattern: option(string),
        maxResults: option(int),
      });

  let handle = (method, args: Yojson.Safe.t) => {
    Base.Result.Let_syntax.(
      switch (method) {
      | "$saveAll" =>
        switch (args) {
        | `List([]) => Ok(SaveAll({includeUntitled: false}))
        | `List([includeUntitledJson]) =>
          let%bind includeUntitled =
            includeUntitledJson
            |> Internal.decode_value(Json.Decode.nullable(Decode.bool));
          Ok(
            SaveAll({
              includeUntitled:
                includeUntitled |> Option.value(~default=false),
            }),
          );
        | _ =>
          Error(
            "Unexpected arguments for $saveAll: "
            ++ Yojson.Safe.to_string(args),
          )
        }

      | "$startFileSearch" =>
        switch (args) {
        | `List([
            includePatternJson,
            _includeFolder,
            excludePatternJson,
            maxResultsJson,
            ..._,
          ]) =>
          let%bind includePattern =
            includePatternJson
            |> Internal.decode_value(Json.Decode.(nullable(string)));
          let%bind excludePattern =
            excludePatternJson
            |> Internal.decode_value(Json.Decode.(nullable(string)));
          let%bind maxResults =
            maxResultsJson
            |> Internal.decode_value(Json.Decode.nullable(Decode.int));

          Ok(StartFileSearch({includePattern, excludePattern, maxResults}));
        | _ =>
          Error(
            "Unexpected arguments for $startFileSearch: "
            ++ Yojson.Safe.to_string(args),
          )
        }

      | _ =>
        Error(
          Printf.sprintf(
            "Unhandled workspace message - %s: %s",
            method,
            Yojson.Safe.to_string(args),
          ),
        )
      }
    );
  };
};

[@deriving show]
type t =
  | Connected
  | Ready
  | Clipboard(Clipboard.msg)
  | Commands(Commands.msg)
  | Configuration(Configuration.msg)
  | Console(Console.msg)
  | DebugService(DebugService.msg)
  | Decorations(Decorations.msg)
  | Diagnostics(Diagnostics.msg)
  | DocumentContentProvider(DocumentContentProvider.msg)
  | Documents(Documents.msg)
  | DownloadService(DownloadService.msg)
  | Errors(Errors.msg)
  | ExtensionService(ExtensionService.msg)
  | FileSystem(FileSystem.msg)
  | LanguageFeatures(LanguageFeatures.msg)
  | Languages(Languages.msg)
  | MessageService(MessageService.msg)
  | OutputService(OutputService.msg)
  | Progress(Progress.msg)
  | QuickOpen(QuickOpen.msg)
  | SCM(SCM.msg)
  | StatusBar(StatusBar.msg)
  | Storage(Storage.msg)
  | Telemetry(Telemetry.msg)
  | TerminalService(TerminalService.msg)
  | Window(Window.msg)
  | Workspace(Workspace.msg)
  | Initialized
  | Disconnected
  | Unhandled
  | Unknown({
      method: string,
      args: Yojson.Safe.t,
    });
