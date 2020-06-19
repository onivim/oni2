open Exthost;

// A format [session] describes a currently in-progress format request.
type session = {
  bufferId: int,
  bufferVersion: int,
  sessionId: int,
};

type documentFormatter = {
  handle: int,
  selector: DocumentSelector.t,
  displayName: string,
};

type model = {
  nextSessionId: int,
  availableDocumentFormatters: list(documentFormatter),
  availableRangeFormatters: list(documentFormatter),
  activeSession: option(session),
};

let initial = {
  nextSessionId: 0,
  availableDocumentFormatters: [],
  availableRangeFormatters: [],
  activeSession: None,
};

[@deriving show]
type command =
  | FormatDocument
  | FormatRange;

[@deriving show]
type msg =
  | Command(command)
  | DocumentFormatterAvailable({
      handle: int,
      selector: Exthost.DocumentSelector.t,
      displayName: string,
    })
  | RangeFormatterAvailable({
      handle: int,
      selector: Exthost.DocumentSelector.t,
      displayName: string,
    })
  | EditsReceived({
      displayName: string,
      sessionId: int,
      edits: [@opaque] list(Vim.Edit.t),
    })
  | EditRequestFailed({
      sessionId: int,
      msg: string,
    })
  | EditCompleted({
      editCount: int,
      displayName: string,
    });

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | FormattingApplied({
      displayName: string,
      editCount: int,
    })
  | FormatError(string);

module Internal = {
  let clearSession = model => {...model, activeSession: None};

  let startSession = (~sessionId, ~buffer, model) => {
    ...model,
    nextSessionId: sessionId + 1,
    activeSession:
      Some({
        sessionId,
        bufferId: Oni_Core.Buffer.getId(buffer),
        bufferVersion: Oni_Core.Buffer.getVersion(buffer),
      }),
  };

let textToArray =
  fun
  | None => [||]
  | Some(text) =>
    text
    |> Utility.StringEx.removeWindowsNewLines
    |> Utility.StringEx.removeTrailingNewLine
    |> Utility.StringEx.splitNewLines;

let extHostEditToVimEdit: Exthost.Edit.SingleEditOperation.t => Vim.Edit.t =
  edit => {
    range: edit.range |> Exthost.OneBasedRange.toRange,
    text: textToArray(edit.text),
  };

  let runFormat = (
    ~formatFn,
    ~model, 
      ~configuration,
        ~matchingFormatters,
          ~buf,
            ~filetype,
              ~extHostClient) => {
      let sessionId = model.nextSessionId;

      let indentation =
        Oni_Core.Indentation.getForBuffer(~buffer=buf, configuration);

      let effects =
        matchingFormatters
        |> List.map(formatter =>
             formatFn(
               ~handle=formatter.handle,
               ~uri=Oni_Core.Buffer.getUri(buf),
               ~options=
                 Exthost.FormattingOptions.{
                   tabSize: indentation.tabSize,
                   insertSpaces:
                     indentation.mode == Oni_Core.IndentationSettings.Spaces,
                 },
               extHostClient,
               res => {
               switch (res) {
               | Ok(edits) =>
                 EditsReceived({
                   displayName: formatter.displayName,
                   sessionId,
                   edits: List.map(extHostEditToVimEdit, edits),
                 })
               | Error(msg) => EditRequestFailed({sessionId, msg})
               }
             })
           )
        |> Isolinear.Effect.batch;

      if (matchingFormatters == []) {
        (
          model,
          FormatError(
            Printf.sprintf("No format providers available for %s", filetype),
          ),
        );
      } else {
        (
          model |> startSession(~sessionId, ~buffer=buf),
          Effect(effects),
        );
      };
  }
};

let update = (~configuration, ~maybeSelection, ~maybeBuffer, ~extHostClient, model, msg) => {
  switch (msg) {
  | Command(FormatRange) =>
    switch((maybeBuffer, maybeSelection)) {
    | (Some(buf), Some(range)) =>
      
      let filetype =
        buf
        |> Oni_Core.Buffer.getFileType
        |> Option.value(~default="plaintext");

      let matchingFormatters =
        model.availableRangeFormatters
        |> List.filter(({selector, _}) =>
             DocumentSelector.matches(~filetype, selector)
           );

  Internal.runFormat(
    ~formatFn=
    Service_Exthost.Effects.LanguageFeatures.provideDocumentRangeFormattingEdits(
      ~range
    ),
    ~model, 
      ~configuration,
        ~matchingFormatters,
          ~buf,
            ~filetype,
              ~extHostClient);
    | _ =>
      (model, FormatError("No range selected."))
    }

  | Command(FormatDocument) =>
    switch (maybeBuffer) {
    | None => (model, Nothing)
    | Some(buf) =>
      let filetype =
        buf
        |> Oni_Core.Buffer.getFileType
        |> Option.value(~default="plaintext");

      let matchingFormatters =
        model.availableDocumentFormatters
        |> List.filter(({selector, _}) =>
             DocumentSelector.matches(~filetype, selector)
           );

  Internal.runFormat(
    ~formatFn=Service_Exthost.Effects.LanguageFeatures.provideDocumentFormattingEdits,
    ~model, 
      ~configuration,
        ~matchingFormatters,
          ~buf,
            ~filetype,
              ~extHostClient);
        
    }
  | DocumentFormatterAvailable({handle, selector, displayName}) => (
      {
        ...model,
        availableDocumentFormatters: [
          {handle, selector, displayName},
          ...model.availableDocumentFormatters,
        ],
      },
      Nothing,
    )

  | RangeFormatterAvailable({handle, selector, displayName}) => (
      {
        ...model,
        availableRangeFormatters: [
          {handle, selector, displayName},
          ...model.availableRangeFormatters,
        ],
      },
      Nothing,
    )

  | EditsReceived({displayName, sessionId, edits}) =>
    switch (model.activeSession) {
    | None => (model, Nothing)
    | Some(activeSession) =>
      // If we received edits for an older session,
      // just ignore.
      if (activeSession.sessionId != sessionId) {
        (model, Nothing);
      } else {
        let effect =
          Service_Vim.Effects.applyEdits(
            ~bufferId=activeSession.bufferId,
            ~version=activeSession.bufferVersion,
            ~edits,
            fun
            | Ok () =>
              EditCompleted({editCount: List.length(edits), displayName})
            | Error(msg) => EditRequestFailed({sessionId, msg}),
          );
        (model, Effect(effect));
      }
    }
  | EditRequestFailed({msg, _}) => (model, FormatError(msg))
  | EditCompleted({displayName, editCount}) => (
      model |> Internal.clearSession,
      FormattingApplied({displayName, editCount}),
    )
  };
};

// COMMANDS

module Commands = {
  open Feature_Commands.Schema;

  let formatDocument =
    define(
      ~category="Formatting",
      ~title="Format Document",
      "editor.action.formatDocument",
      Command(FormatDocument),
    );
};

// CONTRIBUTIONS

module Contributions = {
  let commands = [Commands.formatDocument];
};
