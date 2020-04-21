open Utility;

type direction =
  | Up
  | Left
  | Down
  | Right;

type t('content) =
  | Split([ | `Horizontal | `Vertical], list(t('content)))
  | Window({
      weight: float,
      content: 'content,
    })
  | Empty;

[@deriving show({with_path: false})]
type sizedWindow('content) = {
  content: 'content,
  x: int,
  y: int,
  width: int,
  height: int,
};

module Internal = {
  let intersects = (x, y, split) => {
    x >= split.x
    && x <= split.x
    + split.width
    && y >= split.y
    && y <= split.y
    + split.height;
  };

  let move = (content, dirX, dirY, splits) => {
    let (minX, minY, maxX, maxY, deltaX, deltaY) =
      List.fold_left(
        (prev, cur) => {
          let (minX, minY, maxX, maxY, deltaX, deltaY) = prev;

          let newMinX = cur.x < minX ? cur.x : minX;
          let newMinY = cur.y < minY ? cur.y : minY;
          let newMaxX = cur.x + cur.width > maxX ? cur.x + cur.width : maxX;
          let newMaxY = cur.y + cur.height > maxY ? cur.y + cur.height : maxY;
          let newDeltaX = cur.width / 2 < deltaX ? cur.width / 2 : deltaX;
          let newDeltaY = cur.height / 2 < deltaY ? cur.height / 2 : deltaY;

          (newMinX, newMinY, newMaxX, newMaxY, newDeltaX, newDeltaY);
        },
        (0, 0, 1, 1, 100, 100),
        splits,
      );

    let splitInfo = List.filter(s => s.content == content, splits);

    if (splitInfo == []) {
      None;
    } else {
      let startSplit = List.hd(splitInfo);

      let curX = ref(startSplit.x + startSplit.width / 2);
      let curY = ref(startSplit.y + startSplit.height / 2);
      let found = ref(false);
      let result = ref(None);

      while (! found^
             && curX^ >= minX
             && curX^ < maxX
             && curY^ >= minY
             && curY^ < maxY) {
        let x = curX^;
        let y = curY^;

        let intersects =
          List.filter(
            s => s.content != startSplit.content && intersects(x, y, s),
            splits,
          );

        if (intersects != []) {
          result := Some(List.hd(intersects).content);
          found := true;
        };

        curX := x + dirX * deltaX;
        curY := y + dirY * deltaY;
      };

      result^;
    };
  };

  let rec rotate = (target, func, currenTree) => {
    let findSplit = children => {
      let predicate =
        fun
        | Window({content, _}) => content == target
        | _ => false;

      List.exists(predicate, children);
    };

    switch (currenTree) {
    | Split(direction, children) =>
      Split(
        direction,
        List.map(
          rotate(target, func),
          findSplit(children) ? func(children) : children,
        ),
      )
    | Window(_) as window => window
    | Empty => Empty
    };
  };
};

let initial = Split(`Vertical, [Empty]);

let windows = tree => {
  let rec traverse = (node, acc) => {
    switch (node) {
    | Split(_, children) =>
      List.fold_left((acc, child) => traverse(child, acc), acc, children)
    | Window({content, _}) => [content, ...acc]
    | Empty => acc
    };
  };

  traverse(tree, []);
};

let addWindow = (~target=None, ~position, direction, content, tree) => {
  let newWindow = Window({weight: 1., content});

  let rec f = (targetId, parent, split) => {
    switch (split) {
    | Split(direction, children) => [
        Split(
          direction,
          List.concat(List.map(f(targetId, Some(split)), children)),
        ),
      ]
    | Window({content, _}) as window =>
      if (content == targetId) {
        let children =
          switch (position) {
          | `Before => [newWindow, window]
          | `After => [window, newWindow]
          };

        switch (parent) {
        | Some(Split(dir, _)) =>
          if (dir == direction) {
            children;
          } else {
            [Split(direction, children)];
          }
        | _ => children
        };
      } else {
        [window];
      }
    | Empty => [newWindow]
    };
  };

  switch (target) {
  | Some(targetId) => f(targetId, None, tree) |> List.hd
  | None =>
    switch (tree) {
    | Split(d, children) =>
      Split(d, List.filter(node => node != Empty, [newWindow, ...children]))
    | other => other
    }
  };
};

let rec removeWindow = (target, tree) =>
  switch (tree) {
  | Split(direction, children) =>
    let newChildren =
      children
      |> List.map(child => removeWindow(target, child))
      |> List.filter(node => node != Empty);

    if (List.length(newChildren) > 0) {
      Split(direction, newChildren);
    } else {
      Empty;
    };
  | Window({content, _}) when content == target => Empty
  | Window(_) as window => window
  | Empty => Empty
  };

let rec layout = (x, y, width, height, tree) => {
  switch (tree) {
  | Split(direction, children) =>
    let startX = x;
    let startY = y;
    let count = max(List.length(children), 1);
    let individualWidth = width / count;
    let individualHeight = height / count;

    let result =
      switch (direction) {
      | `Horizontal =>
        List.mapi(
          i =>
            layout(
              startX,
              startY + individualHeight * i,
              width,
              individualHeight,
            ),
          children,
        )
      | `Vertical =>
        List.mapi(
          i =>
            layout(
              startX + individualWidth * i,
              startY,
              individualWidth,
              height,
            ),
          children,
        )
      };

    List.concat(result);
  | Window({content, _}) => [{content, x, y, width, height}]
  | Empty => []
  };
};

let moveCore = (current, dirX, dirY, tree) => {
  let layout = layout(0, 0, 200, 200, tree);

  Internal.move(current, dirX, dirY, layout)
  |> Option.value(~default=current);
};

let moveLeft = current => moveCore(current, -1, 0);
let moveRight = current => moveCore(current, 1, 0);
let moveUp = current => moveCore(current, 0, -1);
let moveDown = current => moveCore(current, 0, 1);

let move = (direction: direction, current, v) => {
  switch (direction) {
  | Up => moveUp(current, v)
  | Down => moveDown(current, v)
  | Left => moveLeft(current, v)
  | Right => moveRight(current, v)
  };
};

let rotateForward = (target, currentTree) => {
  let f =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | list =>
      switch (ListEx.last(list)) {
      | Some(x) => [x, ...ListEx.dropLast(list)]
      | None => []
      };

  Internal.rotate(target, f, currentTree);
};

let rotateBackward = (target, currentTree) => {
  let f =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | [head, ...tail] => tail @ [head];

  Internal.rotate(target, f, currentTree);
};
