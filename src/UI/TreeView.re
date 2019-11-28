open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Model.UiTree;

let defaultNodeStyles = Style.[flexDirection(`Row), marginVertical(5)];

let default = (~indent, {data, status, _}) => {
  let isOpen =
    switch (status) {
    | Open => true
    | Closed => false
    };
  open Style;
  let textStyles = [fontSize(10), color(Colors.black)];
  let indentStr = String.make(indent * 2, ' ');
  let arrow = isOpen ? {||} : {||};
  <Clickable>
    <View style=defaultNodeStyles>
      <Text
        text={indentStr ++ arrow ++ " "}
        style=[fontFamily("FontAwesome5FreeSolid.otf"), ...textStyles]
      />
      <Text
        text=data
        style=[fontFamily("Roboto-Regular.ttf"), ...textStyles]
      />
    </View>
  </Clickable>;
};

/**
 * @param ~indent How much to indent the current (sub)tree.
 * @param ~renderer is a function which determines how each node is rendered
 * @param t The tree to convert to a tree of JSX elements.
 */
let rec renderTree = (~indent=0, ~nodeRenderer, ~emptyRenderer, t) => {
  let drawNode = nodeRenderer(~indent);
  let empty =
    switch (emptyRenderer) {
    | Some(r) => [r(indent)]
    | None => []
    };
  let createSubtree =
    renderTree(~indent=indent + 1, ~nodeRenderer, ~emptyRenderer);
  switch (t) {
  | Empty => empty
  /* If the node is closed OR has no children
     only draw the parent do not render its children */
  | Node({status: Open, _} as x, [])
  | Node({status: Closed, _} as x, _) => [drawNode(x)]
  | Node(current, siblings) =>
    let renderedSiblings =
      List.fold_left(
        (accum, next) => {
          let grandChild = createSubtree(next);
          List.concat([accum, grandChild]);
        },
        [],
        siblings,
      );
    [drawNode(current), ...renderedSiblings];
  };
};

/*
   Cannot set a default argument for the node renderer as this will
   narrow down the type signature of the "tree" to whaterver type the
   default takes making it no longer generalisable
 */
let make = (~children as nodeRenderer, ~tree, ~emptyRenderer=None, ()) => {
  let componentTree = renderTree(tree, ~nodeRenderer, ~emptyRenderer);
  <View> {componentTree |> React.listToElement} </View>;
};
