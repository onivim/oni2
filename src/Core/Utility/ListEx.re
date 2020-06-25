let safeConcat = lists => lists |> List.fold_left(List.append, []);

let safeMap = (f, list) => list |> List.rev |> List.rev_map(f);

/**
 * Return the last element in a list.
 */
let rec last =
  fun
  | [] => None
  | [x] => Some(x)
  | [_, ...t] => last(t);

/**
 * Return all but the last element in a list.
 */
let rec dropLast =
  fun
  | [] => []
  | [_] => []
  | [head, ...tail] => [head, ...dropLast(tail)];

let rec firstk = (k, v) =>
  switch (v) {
  | [] => []
  | [hd, ...tail] =>
    if (k <= 1) {
      [hd];
    } else {
      [hd, ...firstk(k - 1, tail)];
    }
  };

/**
   Get a slice from a list between two indices
 */
let rec sublist = (beginning, terminus, l) =>
  switch (l) {
  | [] => failwith("sublist")
  | [h, ...t] =>
    let tail =
      if (terminus == 0) {
        [];
      } else {
        sublist(beginning - 1, terminus - 1, t);
      };
    if (beginning > 0) {
      tail;
    } else {
      [h, ...tail];
    };
  };

// Like Array.prototype.splice in JavaScript, but for lists
let splice = (~start, ~deleteCount, ~additions, source) => {
  let rec loop = (i, source, deleteCount, additions, acc) =>
    switch (source) {
    | [item, ...rest] when i < start =>
      loop(i + 1, rest, deleteCount, additions, [item, ...acc])
    | [_, ...rest] when deleteCount > 0 =>
      loop(i + 1, rest, deleteCount - 1, additions, acc)
    | _ =>
      acc
      |> List.rev_append(additions)
      |> List.rev_append(source)
      |> List.rev
    };

  loop(0, source, deleteCount, additions, []);
};

let findIndex = (predicate, list) => {
  let rec loop = i =>
    fun
    | [] => None
    | [head, ..._] when predicate(head) => Some(i)
    | [_, ...tail] => loop(i + 1, tail);
  loop(0, list);
};

let rec mergeSortedList = (compareItems, primary, secondary) => {
  switch (primary, secondary) {
  | (_, []) => primary
  | ([], _) => secondary
  | ([headPrimary, ...restPrimary], [headSecondary, ...restSecondary]) =>
    if (compareItems(headPrimary, headSecondary) >= 0) {
      [headPrimary, ...mergeSortedList(compareItems, restPrimary, secondary)];
    } else {
      [headSecondary, ...mergeSortedList(compareItems, primary, restSecondary)];
    }
  };
};
