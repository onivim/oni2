open EditorCoreTypes;

type internalMsg('a) =
  | Nothing
  | ApplyCompletion({
      meetColumn: CharacterIndex.t,
      insertText: string,
    })
  | InsertSnippet({
      meetColumn: CharacterIndex.t,
      snippet: string,
    })
  | OpenFile({
      filePath: string,
      location: option(CharacterPosition.t),
    })
  | ReferencesAvailable
  | NotifySuccess(string)
  | NotifyFailure(string)
  | Effect(Isolinear.Effect.t('a));

let map = f =>
  fun
  | Effect(eff) => Effect(eff |> Isolinear.Effect.map(f))
  | outmsg => outmsg;
