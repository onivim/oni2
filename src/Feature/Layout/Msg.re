[@deriving show({with_path: false})]
type command =
  | NextEditor
  | PreviousEditor
  | SplitVertical
  | SplitHorizontal
  | CloseActiveEditor
  | MoveLeft
  | MoveRight
  | MoveUp
  | MoveDown
  | RotateForward
  | RotateBackward
  | DecreaseSize
  | IncreaseSize
  | DecreaseHorizontalSize
  | IncreaseHorizontalSize
  | DecreaseVerticalSize
  | IncreaseVerticalSize
  | IncreaseWindowSize([ | `Up | `Down | `Left | `Right])
  | DecreaseWindowSize([ | `Up | `Down | `Left | `Right])
  | Maximize
  | MaximizeHorizontal
  | MaximizeVertical
  | ToggleMaximize
  | ResetSizes;

[@deriving show({with_path: false})]
type t =
  | SplitDragged({
      path: list(int),
      delta: float,
    })
  | DragComplete
  | GroupTabClicked(int)
  | GroupSelected(int)
  | EditorCloseButtonClicked(int)
  | Command(command);
