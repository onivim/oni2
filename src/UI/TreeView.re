open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Model.UiTree;

let defaultNodeStyles = Style.[flexDirection(`Row), marginVertical(5)];

let default = (~indent, {data, isOpen, _}) => {
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
let rec renderTree = (~indent=0, ~nodeRenderer, tree) => {
  let drawNode = nodeRenderer(~indent);
  let createSubtree = renderTree(~indent=indent + 1, ~nodeRenderer);
  switch (tree) {
  | {isOpen: true, children: [], _} as x
  | {isOpen: false, _} as x => [drawNode(x)]
  | current =>
    let renderedSiblings =
      List.fold_left(
        (accum, next) => {
          let grandChild = createSubtree(next);
          List.concat([accum, grandChild]);
        },
        [],
        current.children,
      );
    [drawNode(current), ...renderedSiblings];
  };
};

/*
   Cannot set a default argument for the node renderer as this will
   narrow down the type signature of the "tree" to whaterver type the
   default takes making it no longer generalisable
 */
let make = (~children as nodeRenderer, ~tree, ()) => {
  let componentTree = renderTree(tree, ~nodeRenderer);
  <View> {componentTree |> React.listToElement} </View>;
};
