open Exthost;

// A format [session] describes a currently in-progress format request.
type session = { bufferId: int, bufferVersion: int, sessionId: int};
  

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

let initial = {nextSessionId: 0, availableDocumentFormatters: [], session: None};

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
  | EditsReceived({ sessionId: int, edits: list(Oni_Core.SingleEdit.t) })
  | EditRequestFailed({ sessionId: int, msg: string})
  | EditCompleted;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

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
                 Exthost.FormattingOptions.{tabSize: 2, insertSpaces: true},
               extHostClient,
               res => {
                 prerr_endline(
                   "Got edits: " ++ string_of_int(formatter.handle),
                 );
                 switch (res) {
                 | Ok(_edits) => EditsReceived({ sessionId, edits: [] })
                 | Error(msg) => EditRequestFailed({ sessionId, msg })
                 };
                 // TODO: Map result edits to formatting edits
               },
             )
           )
        |> Isolinear.Effect.batch;

      let model' = {
        ...model,
        nextSessionId: sessionId + 1,
        activeSession: Some({
          bufferId: Oni_Core.Buffer.getId(buf),
          bufferVersion: Oni_Core.Buffer.getVersion(buf),
          sessionId,
        })
      };

      (model', Effect(effects));
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
  | EditsReceived({ sessionId, edits}) =>
    // TODO: Handle formatting edits
    let effect = Service_Vim.Effects.applyEdits(
      ~bufferId=0,
      ~version=0,
      ~edits,
      fun
      | Ok() => EditCompleted
      | Error(msg) => EditRequestFailed({ sessionId, msg})
    );
    (model, Effect(effect))
  | EditRequestFailed(msg) =>
    // TODO: Show error notificaiton
    (model, Nothing)
  | EditCompleted =>
  // TODO: Clear format session info
  (model, Nothing)
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
