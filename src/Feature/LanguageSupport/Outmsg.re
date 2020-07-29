open EditorCoreTypes;

type internalMsg('a) =
  | Nothing
  | OpenFile({
      filePath: string,
      location: option(Location.t),
    })
  | NotifySuccess(string)
  | NotifyFailure(string)
  | Effect(Isolinear.Effect.t('a));

let map = f =>
  fun
  | Nothing => Nothing
  | OpenFile(fp) => OpenFile(fp)
  | NotifySuccess(msg) => NotifySuccess(msg)
  | NotifyFailure(msg) => NotifyFailure(msg)
  | Effect(eff) => Effect(eff |> Isolinear.Effect.map(f));
