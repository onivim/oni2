open Oni_Components;

type t('msg) =
  | WriteFailure(MessageBox.model('msg))
  | UnsavedBuffersWarning(MessageBox.model('msg));

[@deriving show({with_path: false})]
type msg =
  | KeyPressed(string);
