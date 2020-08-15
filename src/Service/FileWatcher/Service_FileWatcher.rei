[@deriving show]
type event = {
  path: string,
  hasRenamed: bool,
  hasChanged: bool,
};

let watch: (~path: string, ~onEvent: event => 'msg) => Isolinear.Sub.t('msg);
