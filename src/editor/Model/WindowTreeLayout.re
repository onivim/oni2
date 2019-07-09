open WindowTree;

type t = {
  id: int,
  x: int,
  y: int,
  width: int,
  height: int,
}

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
