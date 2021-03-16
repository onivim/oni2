open EditorCoreTypes;

type internalMsg('a) =
  | Nothing
  | ApplyCompletion({
      replaceSpan: CharacterSpan.t,
      insertText: string,
      additionalEdits: list(Exthost.Edit.SingleEditOperation.t),
    })
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

let map = f =>
  fun
  | Effect(eff) => Effect(eff |> Isolinear.Effect.map(f))
  | outmsg => outmsg;
