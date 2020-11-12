/* FontCache.re
 */
open Revery_Font;

module FontResolutionHash = {
  type t = (Family.t, Weight.t, bool);
  let equal = ((family1, weight1, italic1), (family2, weight2, italic2)) =>
    family1 === family2 && weight1 == weight2 && italic1 == italic2;
  let hash = Hashtbl.hash;
};

module TypefaceResult = {
  type t = FontCache.t;
  let weight = _ => 1;
};

include Lru.M.Make(FontResolutionHash, TypefaceResult);
