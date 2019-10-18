/*
 * MerlinProtcolConverter.re
 *
 * Helpers to convert from merlin protocol types, to the types
 * used for Onivim 2's language services.
 */

module Core = Oni_Core;
module Model = Oni_Model;
open Oni_Extensions;

let toModelDiagnostics = (errors: MerlinProtocol.errorResult) => {
  let f = (err: MerlinProtocol.errorResultItem) => {
    Model.Diagnostics.Diagnostic.create(
      ~message=err.message,
      ~range=
        Core.Range.ofPositions(
          ~startPosition=
            Core.Types.Position.ofInt1(
              err.startPosition.line,
              err.startPosition.col + 1,
            ),
          ~endPosition=
            Core.Types.Position.ofInt1(
              err.endPosition.line,
              err.endPosition.col + 1,
            ),
          (),
        ),
      (),
    );
  };

  List.map(f, errors);
};

let toModelCompletions = (completions: MerlinProtocol.completionResult) => {
  let f = (cmp: MerlinProtocol.completionResultItem) => {
    let ret: Model.Actions.completionItem = {
      completionKind: CompletionKind.Function,
      completionLabel: cmp.name,
      completionDetail: Some(cmp.desc),
    };
    ret;
  };

  List.map(f, completions.entries);
};
