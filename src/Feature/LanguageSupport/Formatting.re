open EditorCoreTypes;
open Exthost;
open Oni_Core;

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
  | FormatSelection;

[@deriving show]
type msg =
  | Command(command)
  | FormatRange({
      startLine: EditorCoreTypes.LineNumber.t,
      endLine: EditorCoreTypes.LineNumber.t,
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

let registerDocumentFormatter = (~handle, ~selector, ~displayName, model) => {
  ...model,
  availableDocumentFormatters: [
    {handle, selector, displayName},
    ...model.availableDocumentFormatters,
  ],
};

let registerRangeFormatter = (~handle, ~selector, ~displayName, model) => {
  ...model,
  availableRangeFormatters: [
    {handle, selector, displayName},
    ...model.availableRangeFormatters,
  ],
};

let unregister = (~handle, model) => {
  ...model,
  availableDocumentFormatters:
    model.availableDocumentFormatters
    |> List.filter(prov => prov.handle != handle),
  availableRangeFormatters:
    model.availableRangeFormatters
    |> List.filter(prov => prov.handle != handle),
};

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

  let fallBackToDefaultFormatter =
      (~indentation, ~languageConfiguration, ~buffer, range: CharacterRange.t) => {
    let lines = buffer |> Oni_Core.Buffer.getLines;

    let startLine = EditorCoreTypes.LineNumber.toZeroBased(range.start.line);
    let stopLine = EditorCoreTypes.LineNumber.toZeroBased(range.stop.line);

    if (startLine >= 0
        && startLine < Array.length(lines)
        && stopLine < Array.length(lines)) {
      let lines = Array.sub(lines, startLine, stopLine - startLine + 1);

      let edits =
        DefaultFormatter.format(
          ~indentation,
          ~languageConfiguration,
          ~startLineNumber=range.start.line,
          lines,
        );

      let displayName = "Default";
      if (edits == []) {
        FormattingApplied({displayName, editCount: 0});
      } else {
        let effect =
          Service_Vim.Effects.applyEdits(
            ~bufferId=buffer |> Oni_Core.Buffer.getId,
            ~version=buffer |> Oni_Core.Buffer.getVersion,
            ~edits,
            fun
            | Ok () =>
              EditCompleted({editCount: List.length(edits), displayName})
            | Error(msg) => EditRequestFailed({sessionId: 0, msg}),
          );
        Effect(effect);
      };
    } else {
      FormatError("Invalid range specified");
    };
  };

  let runFormat =
      (
        ~languageConfiguration,
        ~formatFn,
        ~model,
        ~matchingFormatters,
        ~buf,
        ~extHostClient,
        ~range,
      ) => {
    let sessionId = model.nextSessionId;

    let indentation = Buffer.getIndentation(buf);

    if (matchingFormatters == []) {
      (
        model,
        fallBackToDefaultFormatter(
          ~indentation,
          ~languageConfiguration,
          ~buffer=buf,
          range,
        ),
      );
    } else {
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

      (model |> startSession(~sessionId, ~buffer=buf), Effect(effects));
    };
  };
};

let update =
    (
      ~languageConfiguration,
      ~maybeSelection,
      ~maybeBuffer,
      ~extHostClient,
      msg,
      model,
    ) => {
  switch (msg) {
  | FormatRange({startLine, endLine}) =>
    switch (maybeBuffer) {
    | Some(buf) =>
      let range =
        CharacterRange.{
          start: {
            line: startLine,
            character: CharacterIndex.zero,
          },
          stop: {
            line: endLine,
            character: CharacterIndex.zero,
          },
        };

      let matchingFormatters =
        model.availableRangeFormatters
        |> List.filter(({selector, _}) =>
             DocumentSelector.matchesBuffer(~buffer=buf, selector)
           );

      Internal.runFormat(
        ~languageConfiguration,
        ~formatFn=
          Service_Exthost.Effects.LanguageFeatures.provideDocumentRangeFormattingEdits(
            ~range,
          ),
        ~model,
        ~matchingFormatters,
        ~buf,
        ~extHostClient,
        ~range,
      );
    | None => (model, FormatError("No range selected"))
    }
  | Command(FormatSelection) =>
    switch (maybeBuffer, maybeSelection) {
    | (Some(buf), Some(range)) =>
      let matchingFormatters =
        model.availableRangeFormatters
        |> List.filter(({selector, _}) =>
             DocumentSelector.matchesBuffer(~buffer=buf, selector)
           );

      Internal.runFormat(
        ~languageConfiguration,
        ~formatFn=
          Service_Exthost.Effects.LanguageFeatures.provideDocumentRangeFormattingEdits(
            ~range,
          ),
        ~model,
        ~matchingFormatters,
        ~buf,
        ~extHostClient,
        ~range,
      );
    | _ => (model, FormatError("No range selected."))
    }

  | Command(FormatDocument) =>
    switch (maybeBuffer) {
    | None => (model, Nothing)
    | Some(buf) =>
      let matchingFormatters =
        model.availableDocumentFormatters
        |> List.filter(({selector, _}) =>
             DocumentSelector.matchesBuffer(~buffer=buf, selector)
           );

      Internal.runFormat(
        ~languageConfiguration,
        ~formatFn=Service_Exthost.Effects.LanguageFeatures.provideDocumentFormattingEdits,
        ~model,
        ~matchingFormatters,
        ~buf,
        ~extHostClient,
        ~range=
          EditorCoreTypes.(
            CharacterRange.{
              start: {
                line: LineNumber.zero,
                character: CharacterIndex.zero,
              },
              stop: {
                line:
                  Oni_Core.Buffer.getNumberOfLines(buf)
                  |> LineNumber.ofZeroBased,
                character: CharacterIndex.zero,
              },
            }
          ),
      );
    }

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
