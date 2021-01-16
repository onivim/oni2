open EditorCoreTypes;

type internalMsg('a) =
  | Nothing
  | ApplyCompletion({
      meetColumn: CharacterIndex.t,
      insertText: string,
      additionalEdits: list(Exthost.Edit.SingleEditOperation.t),
    })
  | InsertSnippet({
      meetColumn: CharacterIndex.t,
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
  | Effect(Isolinear.Effect.t('a));

let map = f =>
  fun
  | Effect(eff) => Effect(eff |> Isolinear.Effect.map(f))
  | outmsg => outmsg;
