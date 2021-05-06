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
  | PreviewFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
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
    | Nothing => Nothing
    | ApplyCompletion(v) => ApplyCompletion(v)
    | ApplyWorkspaceEdit(v) => ApplyWorkspaceEdit(v)
    | FormattingApplied(v) => FormattingApplied(v)
    | InsertSnippet(v) => InsertSnippet(v)
    | OpenFile(v) => OpenFile(v)
    | PreviewFile(v) => PreviewFile(v)
    | ReferencesAvailable => ReferencesAvailable
    | NotifySuccess(v) => NotifySuccess(v)
    | NotifyFailure(v) => NotifyFailure(v)
    | CodeLensesChanged(v) => CodeLensesChanged(v)
    | SetSelections(v) => SetSelections(v)
    | TransformConfiguration(v) => TransformConfiguration(v)
    | ShowMenu(v) => ShowMenu(v |> Feature_Quickmenu.Schema.map(f))
    | Effect(eff) => Effect(eff |> Isolinear.Effect.map(f));
