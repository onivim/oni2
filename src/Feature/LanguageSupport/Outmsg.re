open Oni_Core;
open EditorCoreTypes;

type internalMsg('a) =
  | Nothing
  | ApplyCompletion({
      replaceSpan: CharacterSpan.t,
      insertText: string,
      additionalEdits: list(Exthost.Edit.SingleEditOperation.t),
    })
  | ApplyWorkspaceEdit(Exthost.WorkspaceEdit.t)
  | FormattingApplied({
      displayName: string,
      editCount: int,
      needsToSave: bool,
    })
  | InsertSnippet({
      replaceSpan: CharacterSpan.t,
      snippet: string,
      additionalEdits: list(Exthost.Edit.SingleEditOperation.t),
    })
  | OpenFile({
      filePath: string,
      location: option(CharacterPosition.t),
      direction: SplitDirection.t,
    })
  | ReferencesAvailable
  | NotifySuccess(string)
  | NotifyFailure(string)
  | CodeLensesChanged({
      handle: int,
      bufferId: int,
      startLine: EditorCoreTypes.LineNumber.t,
      stopLine: EditorCoreTypes.LineNumber.t,
      lenses: list(CodeLens.codeLens),
    })
  | SetSelections({
      editorId: int,
      ranges: list(CharacterRange.t),
    })
  | ShowMenu(Feature_Quickmenu.Schema.menu('a))
  | TransformConfiguration(Oni_Core.ConfigurationTransformer.t)
  | Effect(Isolinear.Effect.t('a));

let map: ('a => 'b, internalMsg('a)) => internalMsg('b) =
  f =>
    fun
    // These are the polymorphic items...
    | ShowMenu(menu) => ShowMenu(menu |> Feature_Quickmenu.Schema.map(f))
    | Effect(eff) => Effect(eff |> Isolinear.Effect.map(f))
    // ...and for some reason the remaining have to be explicitly specified
    | Nothing => Nothing
    | ApplyCompletion(orig) => ApplyCompletion(orig)
    | FormattingApplied(orig) => FormattingApplied(orig)
    | ApplyWorkspaceEdit(edit) => ApplyWorkspaceEdit(edit)
    | InsertSnippet(edit) => InsertSnippet(edit)
    | OpenFile(orig) => OpenFile(orig)
    | ReferencesAvailable => ReferencesAvailable
    | NotifySuccess(str) => NotifySuccess(str)
    | NotifyFailure(str) => NotifyFailure(str)
    | CodeLensesChanged(orig) => CodeLensesChanged(orig)
    | SetSelections(orig) => SetSelections(orig)
    | TransformConfiguration(orig) => TransformConfiguration(orig);
