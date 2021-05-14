open Revery;
open Oni_Core;

module BoundingBox2d = Revery.Math.BoundingBox2d;

module Effects = {
  let setIMEPosition: BoundingBox2d.t => Isolinear.Effect.t(_) =
    bbox => {
      Isolinear.Effect.create(~name="Service_IME.Effects.setImePosition", () => {
        let (x0, y0, x1, y1) = BoundingBox2d.getBounds(bbox);
        Sdl2.TextInput.setInputRect(
          x0 |> int_of_float,
          y0 |> int_of_float,
          x1 -. x0 |> int_of_float,
          y1 -. y0 |> int_of_float,
        );
      });
    };
};

module Sub = {
  type edit = {
    text: string,
    start: int,
    length: int,
  };

  let ime =
      (~window as _, ~compositionStart, ~compositionEdit, ~compositionEnd) => Isolinear.Sub.none;
};
