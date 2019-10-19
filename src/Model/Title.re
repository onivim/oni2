/*
 * Title.re
 *
 * Model for working with the window title
 */


type t =
| Text(string)
| Separator
| Variable(string);

let regexp = Str.regexp("\\${\\([a-z]+\\)}");

let ofString = str => {
  let idx = ref(0);
  let len = String.length(str);
  let result = ref([]);
  while (idx^ < len) {
    switch(Str.search_forward(regexp, str, idx^)) {
    | exception Not_found => 
      result := [Text(String.sub(str, idx^, len - idx^)), ...result^];
      idx := len
    | v => 
      let prev = v - idx^;

      if (prev > 0) {
        result := [Text(String.sub(str, idx^, prev)), ...result^];
      }

      let group = Str.matched_group(1, str);
      idx := v + String.length(group) + 3;
      if (String.equal(group, "separator")) {
      result := [Separator, ...result^];
      } else {
      result := [Variable(group), ...result^];
      }
    }
  }

  result^ |> List.rev;
};
