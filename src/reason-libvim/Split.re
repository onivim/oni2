type t =
  | NewHorizontal
  | Horizontal({filePath: option(string)})
  | NewVertical
  | Vertical({filePath: option(string)})
  | NewTabPage
  | TabPage({filePath: option(string)});

let ofNative = (splitType: Types.windowSplitType, path: string) => {
  let hasPath = path != "";
  switch (splitType) {
  // Horizontal
  | Types.HorizontalNew when !hasPath => NewHorizontal
  | Types.HorizontalNew when hasPath => Horizontal({filePath: Some(path)})
  | Types.Horizontal when !hasPath => Horizontal({filePath: None})
  | Types.Horizontal when hasPath => Horizontal({filePath: Some(path)})

  // Vertical
  | Types.VerticalNew when !hasPath => NewVertical
  | Types.VerticalNew when hasPath => Vertical({filePath: Some(path)})
  | Types.Vertical when !hasPath => Vertical({filePath: None})
  | Types.Vertical when hasPath => Vertical({filePath: Some(path)})

  // TabPage
  | Types.TabPageNew when !hasPath => NewTabPage
  | Types.TabPageNew when hasPath => TabPage({filePath: Some(path)})
  | Types.TabPage when !hasPath => TabPage({filePath: None})
  | Types.TabPage when hasPath => TabPage({filePath: Some(path)})

  | _ =>
    // This should never be hit, but the compiler doesn't know the above check is exhaustive
    NewHorizontal
  };
};
