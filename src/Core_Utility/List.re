include Stdlib.List;

let filter_map = f => {
  let rec aux = accu =>
    fun
    | [] => rev(accu)
    | [x, ...l] =>
      switch (f(x)) {
      | None => aux(accu, l)
      | Some(v) => aux([v, ...accu], l)
      };

  aux([]);
};
