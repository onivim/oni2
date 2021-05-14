open Revery;
open Oni_Core;

module Sub = {
  type edit = {
    text: string,
    start: int,
    length: int,
  };

  let ime =
      (~window as _, ~compositionStart, ~compositionEdit, ~compositionEnd) => Isolinear.Sub.none;
};
