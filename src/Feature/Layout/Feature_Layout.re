open Utility;

type direction =
  | Up
  | Left
  | Down
  | Right;

[@deriving show({with_path: false})]
type t('id) =
  | Split([ | `Horizontal | `Vertical], list(container('id)))
  | Window('id)
  | Empty
and container('id) = {
  weight: float,
  content: t('id),
};

[@deriving show({with_path: false})]
type sizedWindow('id) = {
  id: 'id,
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

  let move = (id, dirX, dirY, splits) => {
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

    let splitInfo = List.filter(s => s.id == id, splits);

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
            s => s.id != startSplit.id && intersects(x, y, s),
            splits,
          );

        if (intersects != []) {
          result := Some(List.hd(intersects).id);
          found := true;
        };

        curX := x + dirX * deltaX;
        curY := y + dirY * deltaY;
      };

      result^;
    };
  };

  let rec rotate = (target, func, tree) => {
    let findSplit = containers => {
      let predicate =
        fun
        | {content: Window(id), _} => id == target
        | _ => false;

      List.exists(predicate, containers);
    };

    switch (tree) {
    | Split(direction, containers) =>
      Split(
        direction,
        List.map(
          container =>
            {...container, content: rotate(target, func, container.content)},
          findSplit(containers) ? func(containers) : containers,
        ),
      )
    | Window(_) as window => window
    | Empty => Empty
    };
  };
};

let initial = Empty;

let windows = tree => {
  let rec traverse = (node, acc) => {
    switch (node) {
    | Split(_, containers) =>
      List.fold_left(
        (acc, container) => traverse(container.content, acc),
        acc,
        containers,
      )
    | Window(id) => [id, ...acc]
    | Empty => acc
    };
  };

  traverse(tree, []);
};

let addWindow = (~target=None, ~position, direction, id, tree) => {
  let newContainer = {weight: 1., content: Window(id)};
  switch (target) {
  | Some(targetId) =>
    let rec traverse = node => {
      switch (node) {
      | Split(thisDirection, containers) when thisDirection == direction =>
        let onMatch = container =>
          switch (position) {
          | `Before => [newContainer, container]
          | `After => [container, newContainer]
          };
        Split(thisDirection, traverseContainers(~onMatch, [], containers));

      | Split(thisDirection, containers) =>
        let onMatch = container =>
          switch (position) {
          | `Before => [
              {
                weight: container.weight,
                content:
                  Split(
                    direction,
                    [newContainer, {...container, weight: 1.}],
                  ),
              },
            ]
          | `After => [
              {
                weight: container.weight,
                content:
                  Split(
                    direction,
                    [{...container, weight: 1.}, newContainer],
                  ),
              },
            ]
          };
        Split(thisDirection, traverseContainers(~onMatch, [], containers));

      | Window(id) as window when id == targetId =>
        switch (position) {
        | `Before =>
          Split(direction, [newContainer, {weight: 1., content: window}])
        | `After =>
          Split(direction, [{weight: 1., content: window}, newContainer])
        }

      | Window(_) as window => window

      | Empty => Window(id)
      };
    }

    and traverseContainers = (~onMatch, before, after) =>
      switch (after) {
      | [] => List.rev(before)
      | [head, ...rest] =>
        switch (head) {
        | {content: Window(id), _} as container when id == targetId =>
          traverseContainers(~onMatch, onMatch(container) @ before, rest)

        | {content: Split(_) as split, _} as container =>
          traverseContainers(
            ~onMatch,
            [{...container, content: traverse(split)}, ...before],
            rest,
          )
        | container =>
          traverseContainers(~onMatch, [container, ...before], rest)
        }
      };

    traverse(tree);

  | None =>
    switch (tree) {
    | Split(d, containers) => Split(d, [newContainer, ...containers])
    | _ => tree
    }
  };
};

let rec removeWindow = (target, tree) =>
  switch (tree) {
  | Split(direction, containers) =>
    let newContainers =
      containers
      |> List.map(container =>
           {...container, content: removeWindow(target, container.content)}
         )
      |> List.filter(({content, _}) => content != Empty);

    if (newContainers == []) {
      Empty;
    } else {
      Split(direction, newContainers);
    };
  | Window(id) when id == target => Empty
  | node => node
  };

let rec layout = (x, y, width, height, tree) => {
  // Console.log(
  //   show((fmt, id) => Format.pp_print_int(fmt, Obj.magic(id)), tree),
  // );
  switch (tree) {
  | Split(direction, containers) =>
    let totalWeight =
      containers
      |> List.map(container => container.weight)
      |> List.fold_left((+.), 0.)
      |> max(1.);

    (
      switch (direction) {
      | `Horizontal =>
        let unitHeight = float(height) /. totalWeight;
        List.fold_left(
          ((y, acc), container) => {
            let height = int_of_float(unitHeight *. container.weight);
            let windows = layout(x, y, width, height, container.content);
            (y + height, windows @ acc);
          },
          (y, []),
          containers,
        );

      | `Vertical =>
        let unitWidth = float(width) /. totalWeight;
        List.fold_left(
          ((x, acc), container) => {
            let width = int_of_float(unitWidth *. container.weight);
            let windows = layout(x, y, width, height, container.content);
            (x + width, windows @ acc);
          },
          (x, []),
          containers,
        );
      }
    )
    |> snd
    |> List.rev;

  | Window(id) =>
    // Printf.printf("%n: %n, %n, %n, %n\n%!", Obj.magic(id), x, y, width, height);
    [{id, x, y, width, height}]
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

let rotateForward = (target, tree) => {
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

  Internal.rotate(target, f, tree);
};

let rotateBackward = (target, tree) => {
  let f =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | [head, ...tail] => tail @ [head];

  Internal.rotate(target, f, tree);
};

