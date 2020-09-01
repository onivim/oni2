open Oni_Core;

[@deriving show]
type t = (int, int);

let decode = {
  Json.Decode.(
    list(int)
    |> and_then(
         fun
         | [cacheId0, cacheId1] => succeed((cacheId0, cacheId1))
         | _ => fail("Expected (int, int)"),
       )
  );
};

let encode = {
  Json.Encode.(
    ((cacheId0, cacheId1)) => {
      [cacheId0, cacheId1] |> list(int);
    }
  );
};
