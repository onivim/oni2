/*
 Trie.re

 Simple Trie implementation to support scope selection
 */

type t('a) = {
  v: option('a),
  prefix: string,
  children: StringMap.t(t('a)),
};

let empty: t('a) = {v: None, prefix: "", children: StringMap.empty};

let rec update =
        (path: list(string), f: option('a) => option('a), tree: t('a)) => {
  switch (path) {
  | [hd, ...tail] =>
    let recurse = (childTree: option(t('a))) => {
      switch (childTree) {
      | None => Some(update(tail, f, {...empty, prefix: hd}))
      | Some(v) => Some(update(tail, f, v))
      };
    };

    let children = StringMap.update(hd, recurse, tree.children);
    {...tree, children};
  | [] =>
    let newVal = f(tree.v);
    {...tree, v: newVal};
  };
};

let show = (printer: 'a => string, root: t('a)) => {
  let rec f = (indentation, tree: t('a)) => {
    let pp =
      switch (tree.v) {
      | None => "None"
      | Some(v) => "Some(" ++ printer(v) ++ ")"
      };

    let indent = String.make(indentation, ' ');

    let s = "\n" ++ indent ++ "(" ++ tree.prefix ++ " : " ++ pp;
    let m =
      List.fold_left(
        (prev, cur) => {
          let (_, v) = cur;
          prev ++ f(indentation + 1, v);
        },
        "",
        StringMap.bindings(tree.children),
      );

    s ++ m ++ ")\n";
  };

  f(0, root);
};

let matches = (tree: t('a), path: list(string)) => {
  let rec f =
          (
            tree: t('a),
            path: list(string),
            curr: list((string, option('a))),
          ) => {
    switch (path) {
    | [hd, ...tail] =>
      switch (StringMap.find_opt(hd, tree.children)) {
      | Some(v) => f(v, tail, [(hd, v.v), ...curr])
      | None => curr
      }
    | [] => curr
    };
  };

  f(tree, path, []);
};
