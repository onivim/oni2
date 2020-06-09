open Exthost;

type documentFormatter = {
  handle: int,
  selector: DocumentSelector.t,
  displayName: string,
};

type model = {availableDocumentFormatters: list(documentFormatter)};

let initial = {availableDocumentFormatters: []};

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
    });

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
               _res
               // TODO: Map back to formatting edits
               // Json parse error?
               => failwith("Got edits: " ++ string_of_int(formatter.handle)))
           )
        |> Isolinear.Effect.batch;

      (model, Effect(effects));
    }
  | DocumentFormatterAvailable({handle, selector, displayName}) => (
      {
        availableDocumentFormatters: [
          {handle, selector, displayName},
          ...model.availableDocumentFormatters,
        ],
      },
      Nothing,
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
