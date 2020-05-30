/*
 ColorTheme.re
 */

type t = StringMap.t(string);

let empty = StringMap.empty;

let of_map = v => v;

let of_yojson = json => {
  switch (json) {
  | `Assoc(colorMap) =>
    List.fold_left(
      (prev, curr) => {
        let (colorKey, jsonValue) = curr;

        switch (jsonValue) {
        | `String(color) => StringMap.add(colorKey, color, prev)
        | _ => prev
        };
      },
      empty,
      colorMap,
    )
  | _ => empty
  };
};

let of_plist =
  Plist.(
    assoc(string) |> map(colors => colors |> List.to_seq |> StringMap.of_seq)
  );

let union = (a, b) => {
  let f = (_key, v1, _v2) => {
    Some(v1);
  };

  StringMap.union(f, b, a);
};

let getColor = (name, v) => StringMap.find_opt(name, v);

let getFirstOrDefault = (~default, candidates, v) => {
  let rec f = curr =>
    switch (curr) {
    | [] => default
    | [hd, ...tail] =>
      switch (getColor(hd, v)) {
      | Some(col) => col
      | None => f(tail)
      }
    };

  f(candidates);
};

let fold = StringMap.fold;
