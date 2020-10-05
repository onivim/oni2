open Oni_Core;

type t =
  | NoWrap
  | Viewport
  | WrapColumn(int)
  | Bounded(int);

let fromConfig = (config: Config.resolver) => {
  let wordWrap = EditorConfiguration.Experimental.wordWrap.get(config);
  let wordWrapColumn =
    EditorConfiguration.Experimental.wordWrapColumn.get(config);

  switch (wordWrap) {
  | `On => Viewport
  | `Off => NoWrap
  | `WordWrapColumn => WrapColumn(wordWrapColumn)
  | `Bounded => Bounded(wordWrapColumn)
  };
};
