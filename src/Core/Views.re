/**
 This type module represents the various "views" of vim
  each of which can be interacted in their own ways

  * A Tab: contains windows
  * A Window: contains buffers
  * A buffer: represents a file
*/

[@deriving show({with_path: false})]
type t =
  | Window
  | Tab
  | Buffer;

type openMethod =
  | Buffer
  | Tab;

[@deriving show({with_path: false})]
type viewOperation =
  (~path: string=?, ~id: int=?, ~openMethod: openMethod=?, unit) => unit;
