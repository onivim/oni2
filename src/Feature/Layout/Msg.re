[@deriving show({with_path: false})]
type command =
  | PreviousEditor
  | NextEditor
  | SplitVertical
  | SplitHorizontal
  | CloseActiveEditor
  | CloseActiveGroup
  | MoveLeft
  | MoveRight
  | MoveUp
  | MoveDown
  | MoveTopLeft
  | MoveBottomRight
  | CycleForward
  | CycleBackward
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
  | ResetSizes
  | AddLayout
  | PreviousLayout
  | NextLayout;

[@deriving show({with_path: false})]
type t =
  | SplitDragged({
      path: list(int),
      delta: float,
    })
  | DragComplete
  | EditorTabClicked({
      groupId: int,
      editorId: int,
    })
  | EditorTabDoubleClicked({
      groupId: int,
      editorId: int,
    })
  | GroupSelected(int)
  | EditorCloseButtonClicked(int)
  | LayoutTabClicked(int)
  | LayoutCloseButtonClicked(int)
  | Command(command);
