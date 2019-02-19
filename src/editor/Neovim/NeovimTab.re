open Oni_Core;
open Types;

module M = Msgpck;
/*
   Msgpck has a special type for Maps, which returns a tuple
   consisting of the key and the value, so we pattern
   match on the name and value and use those to construct
   the record type we want
 */
let parseTabMap = map =>
  Tabline.(
    List.fold_left(
      (accum, item) =>
        switch (item) {
        | (M.String("name"), M.String(value)) => {...accum, name: value}
        | (M.String("tab"), value) =>
          let tab =
            switch (Utility.convertNeovimExtType(value)) {
            | Some((_, id)) => id
            | None => 0
            };
          {...accum, tab};
        | _ => accum
        },
      {name: "", tab: 0},
      map,
    )
  );

let parseTablineUpdate = msgs =>
  /*
     The structure of a tabline update is -
     [tabline_update, [(2 "\001"), [{tab: (2 "\001"), name: string}]]],
   */
  switch (msgs) {
  | [_view, M.List(tabs)] =>
    List.fold_left(
      (accum, tab) =>
        switch (tab) {
        | M.Map(map) =>
          let map = parseTabMap(map);
          [Tabline.{tab: map.tab, name: map.name}, ...accum];
        | _ => accum
        },
      [],
      tabs,
    )
  | _ => []
  };
