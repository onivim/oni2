[@deriving show({with_path: false})]
type msg =
  | FilesDroppedOnEditor({paths: list(string)});
