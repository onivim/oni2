open Revery;
open Oni_Core;

module Sub: {
  type edit = {
    text: string,
    start: int,
    length: int,
  };

  let ime:
    (
      ~window: Window.t,
      ~compositionStart: 'msg,
      ~compositionEdit: edit => 'msg,
      ~compositionEnd: 'msg
    ) =>
    Isolinear.Sub.t('msg);
};
