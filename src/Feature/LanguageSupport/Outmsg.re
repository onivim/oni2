open EditorCoreTypes;

type internalMsg('a) =
  | Nothing
  | ApplyCompletion({
      meetColumn: Index.t,
      insertText: string,
    })
  | InsertSnippet({
      meetColumn: Index.t,
      snippet: string,
    })
  | OpenFile({
      filePath: string,
      location: option(Location.t),
    })
  | NotifySuccess(string)
  | NotifyFailure(string)
  | Effect(Isolinear.Effect.t('a));

let map = f =>
  fun
  | Effect(eff) => Effect(eff |> Isolinear.Effect.map(f))
  | outmsg => outmsg;
