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
  activeSession: option(session),
};

let initial = {
  nextSessionId: 0,
  availableDocumentFormatters: [],
  activeSession: None,
};

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
};

[@deriving show]
type command =
  | FormatDocument;

[@deriving show]
type msg =
  | Command(command)
  | DocumentFormatterAvailable({
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

let textToArray =
  fun
  | None => [||]
  | Some(text) =>
    text
    |> Utility.StringEx.removeWindowsNewLines
    |> Utility.StringEx.splitNewLines;

let extHostEditToVimEdit: Exthost.Edit.SingleEditOperation.t => Vim.Edit.t =
  edit => {
    range: edit.range |> Exthost.OneBasedRange.toRange,
    text: textToArray(edit.text),
  };

let update = (~maybeBuffer, ~extHostClient, model, msg) => {
  switch (msg) {
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
      let sessionId = model.nextSessionId;

      let effects =
        matchingFormatters
        |> List.map(formatter =>
             Service_Exthost.Effects.LanguageFeatures.provideDocumentFormattingEdits(
               ~handle=formatter.handle,
               ~uri=Oni_Core.Buffer.getUri(buf),
               // TODO: Hook up to indentation settings
               ~options=
                 Exthost.FormattingOptions.{tabSize: 2, insertSpaces: false},
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
          model |> Internal.startSession(~sessionId, ~buffer=buf),
          Effect(effects),
        );
      };
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
