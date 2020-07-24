open EditorCoreTypes;

type outmsg =
  | Nothing
  | OpenFile({
      filePath: string,
      location: option(Location.t),
    });
