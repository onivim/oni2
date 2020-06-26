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

/**
 * removeAt
 */
let removeAt = (index, list) => {
  let left = Base.List.take(list, index);
  let right = Base.List.drop(list, index + 1);
  left @ right;
};

let%test_module "removeAt" =
  (module
   {
     let%test "0" = removeAt(0, [1, 2, 3]) == [2, 3];
     let%test "1" = removeAt(1, [1, 2, 3]) == [1, 3];
     let%test "2" = removeAt(2, [1, 2, 3]) == [1, 2];
     let%test "-10 (out of bounds)" = removeAt(-10, [1, 2, 3]) == [1, 2, 3];
     let%test "10 (out of bounds)" = removeAt(10, [1, 2, 3]) == [1, 2, 3];
   });

/**
 * insertAt
 */
let insertAt = (index, element, list) => {
  let (left, right) = Base.List.split_n(list, index);
  left @ [element] @ right;
};

let%test_module "removeAt" =
  (module
   {
     let%test "0" = insertAt(0, 42, [1, 2, 3]) == [42, 1, 2, 3];
     let%test "1" = insertAt(1, 42, [1, 2, 3]) == [1, 42, 2, 3];
     let%test "2" = insertAt(2, 42, [1, 2, 3]) == [1, 2, 42, 3];
     let%test "3" = insertAt(3, 42, [1, 2, 3]) == [1, 2, 3, 42];
     let%test "-10 (out of bounds)" =
       insertAt(-10, 42, [1, 2, 3]) == [42, 1, 2, 3];
     let%test "10 (out of bounds)" =
       insertAt(10, 42, [1, 2, 3]) == [1, 2, 3, 42];
   });

/**
 * move
 */
let move = (~fromi, ~toi, list) => {
  let element = List.nth(list, fromi);
  list |> removeAt(fromi) |> insertAt(toi, element);
};

let%test_module "move" =
  (module
   {
     let%test "0 -> 0" = move(~fromi=0, ~toi=0, [1, 2, 3]) == [1, 2, 3];
     let%test "0 -> 1" = move(~fromi=0, ~toi=1, [1, 2, 3]) == [2, 1, 3];
     let%test "0 -> 2" = move(~fromi=0, ~toi=2, [1, 2, 3]) == [2, 3, 1];
     let%test "1 -> 0" = move(~fromi=1, ~toi=0, [1, 2, 3]) == [2, 1, 3];
     let%test "1 -> 1" = move(~fromi=1, ~toi=1, [1, 2, 3]) == [1, 2, 3];
     let%test "1 -> 2" = move(~fromi=1, ~toi=2, [1, 2, 3]) == [1, 3, 2];
     let%test "2 -> 0" = move(~fromi=2, ~toi=0, [1, 2, 3]) == [3, 1, 2];
     let%test "2 -> 1" = move(~fromi=2, ~toi=1, [1, 2, 3]) == [1, 3, 2];
     let%test "2 -> 2" = move(~fromi=2, ~toi=2, [1, 2, 3]) == [1, 2, 3];
     let%test "-10 -> 1 (out of bounds)" =
       switch (move(~fromi=-10, ~toi=1, [1, 2, 3])) {
       | exception (Invalid_argument(_)) => true
       | _ => false
       };
     let%test "10 -> 1 (out of bounds)" =
       switch (move(~fromi=10, ~toi=1, [1, 2, 3])) {
       | exception (Failure(_)) => true
       | _ => false
       };
   });
