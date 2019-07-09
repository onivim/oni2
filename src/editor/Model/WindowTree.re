module WindowSplitId =
  Revery.UniqueId.Make({});

[@deriving show({with_path: false})]
type direction =
  | Horizontal
  | Vertical;

[@deriving show({with_path: false})]
type split = {
  id: int,
  editorGroupId: int,
  width: option(int),
  height: option(int),
};

[@deriving show({with_path: false})]
type t =
  | Parent(direction, list(t))
  | Leaf(split)
  | Empty;

let empty = Parent(Vertical, [Empty]);

let createSplit = (~width=?, ~height=?, ~editorGroupId, ()) => {
  id: WindowSplitId.getUniqueId(),
  editorGroupId,
  width,
  height,
};

let filterEmpty = v =>
  switch (v) {
  | Empty => false
  | _ => true
  };

let addSplit = (~target=None, direction, newSplit, currentTree) => {
  let rec f = (targetId, parent, split) => {
    switch (split) {
    | Parent(d, tree) => [
        Parent(d, List.concat(List.map(f(targetId, Some(split)), tree))),
      ]
    | Leaf(v) =>
      if (v.id == targetId) {
        switch (parent) {
        | Some(Parent(dir, _)) =>
          if (dir == direction) {
            prerr_endline("leaf found parent equal");
            [Leaf(newSplit), Leaf(v)];
          } else {
            prerr_endline("leaf found parent not equal");
            [Parent(direction, [Leaf(newSplit), Leaf(v)])];
          }
        | _ =>
          prerr_endline("other case");
          [Leaf(newSplit), Leaf(v)];
        };
      } else {
        [Leaf(v)];
      }
    | Empty => [Leaf(newSplit)]
    };
  };

  switch (target) {
  | Some(targetId) => f(targetId, None, currentTree) |> List.hd
  | None =>
    switch (currentTree) {
    | Parent(d, tree) =>
      Parent(d, List.filter(filterEmpty, [Leaf(newSplit), ...tree]))
    | v => v
    }
  };
};

let rec removeSplit = (id, currentTree) =>
  switch (currentTree) {
  | Parent(direction, children) =>
    let newChildren = List.map(child => removeSplit(id, child), children);
    Parent(direction, newChildren);
  | Leaf(split) when split.id == id => Empty
  | Leaf(_) as leaf => leaf
  | Empty => Empty
  };
 
let rec layout = (x: int, y: int, width: int, height: int, tree: t) => {

  switch (tree) {
  | Parent(direction, children) => {
        let count = List.length(children);
        let individualWidth = width / count;
        let individualHeight = height / count;

    let result = switch (direction) {
    | Horizontal => {
        List.map((i) => layout(startX, startY + individualHeight * i, width, individualHeight), children);
      }
    | Vertical => {
        List.map((i) => layout(startX + individualWidth * i, startY, individualWidth, height), children);
    }
    }

    List.concat(result);
  }
  | Leaf({id, _}) => [(id, x, y, width, height)]
  | Empty => []
  }


};
