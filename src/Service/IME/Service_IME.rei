open Revery;
open Oni_Core;

module Effects: {
  let setIMEPosition: Math.BoundingBox2d.t => Isolinear.Effect.t(_);
};

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
