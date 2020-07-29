open Oni_Core;
[@deriving show]
type t = {
  label: string,
  kind: CompletionKind.t,
  detail: option(string),
  documentation: option(string),
  sortText: option(string),
  filterText: option(string),
  insertText: option(string),
  // TODO:
  // insertTextRules
  // range
  // commitCharacters
  // additionalTextEdits
  // command
  // kindModifer
  // chainedCacheId?
};

let filterText = ({filterText, label, _}) => {
  switch (filterText) {
  | None => label
  | Some(filter) => filter
  };
};

let sortText = ({sortText, label, _}) => {
  switch (sortText) {
  | None => label
  | Some(sortText) => sortText
  };
};

let decode = {
  Json.Decode.(
    obj(({field, _}) => {
      // These fields come from the `ISuggestDataDtoField` definition:
      // https://github.com/onivim/vscode-exthost/blob/50bef147f7bbd250015361a4e3cad3305f65bc27/src/vs/workbench/api/common/extHost.protocol.ts#L1089
      let label = field.required("a", string);

      let kind =
        field.required("b", int)
        |> CompletionKind.ofInt
        |> Option.value(~default=CompletionKind.Method);

      let detail = field.optional("c", string);
      // TODO: Handle parsing correctly
      // There are other types besides string that documentation can take..
      //let documentation = field.optional("d", string);
      let sortText = field.optional("e", string);
      let filterText = field.optional("f", string);
      let insertText = field.optional("h", string);
      {
        label,
        kind,
        detail,
        documentation: None,
        sortText,
        filterText,
        insertText,
      };
    })
  );
};
