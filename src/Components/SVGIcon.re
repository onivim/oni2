type t =
  (~size: int=?, ~strokeWidth: int=?, ~color: Revery.Color.t=?, unit) =>
  Revery.UI.element;

let colorToHex = color => {
  let (r, g, b, _) = Revery.Color.toRgba(color);
  Printf.sprintf(
    "#%02X%02X%02X",
    int_of_float(r *. 255.),
    int_of_float(g *. 255.),
    int_of_float(b *. 255.),
  );
};
