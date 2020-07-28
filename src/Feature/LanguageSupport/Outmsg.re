open EditorCoreTypes;

type internalMsg('a) =
  | Nothing
  | OpenFile({
      filePath: string,
      location: option(Location.t),
    })
  | Effect(Isolinear.Effect.t('a));

let map = f =>
  fun
  | Nothing => Nothing
  | OpenFile(fp) => OpenFile(fp)
  | Effect(eff) => Effect(eff |> Isolinear.Effect.map(f));
