open Revery;
open Revery.UI;
open Revery.UI.Components;

module type TreeModel = {
  type t;

  let children: t => [ | `Loading | `Loaded(list(t))];
  let kind: t => [ | `Node([ | `Open | `Closed]) | `Leaf];
};

module Constants = {
  let arrowSize = 15;
  let indentSize = 12.;
};

module Styles = {
  open Style;

  let clickable = [
    cursor(Revery.MouseCursors.pointer),
    flexDirection(`Row),
    marginVertical(3),
  ];
  let children = [transform(Transform.[TranslateX(Constants.indentSize)])];

  let loading = [fontFamily("Arial"), fontSize(10)];
};

let arrow = (~isOpen, ()) =>
  <FontIcon
    fontSize=Constants.arrowSize
    color=Colors.white
    icon={isOpen ? FontAwesome.caretDown : FontAwesome.caretRight}
    backgroundColor=Colors.transparentWhite
  />;

let noArrow = () =>
  <View
    style=Style.[width(Constants.arrowSize), height(Constants.arrowSize)]
  />;

let rec nodeView:
  type node.
    (
      ~renderContent: node => React.element(React.node),
      ~model: (module TreeModel with type t = node),
      ~onClick: node => unit,
      ~node: node,
      unit
    ) =>
    React.element(React.node) =
  (~renderContent, ~model, ~onClick, ~node, ()) => {
    let (module Model) = model;

    switch (Model.kind(node)) {
    | `Node(state) =>
      <View>
        <Clickable onClick={() => onClick(node)} style=Styles.clickable>
          <arrow isOpen={state == `Open} />
          {renderContent(node)}
        </Clickable>
        <View style=Styles.children>
          {switch (state) {
           | `Open =>
             switch (Model.children(node)) {
             | `Loading => <Text text="Loading..." style=Styles.loading />

             | `Loaded(children) =>
               children
               |> List.map(child =>
                    <nodeView renderContent model onClick node=child />
                  )
               |> React.listToElement
             }

           | `Closed => React.empty
           }}
        </View>
      </View>

    | `Leaf =>
      <View>
        <Clickable onClick={() => onClick(node)} style=Styles.clickable>
          <noArrow />
          {renderContent(node)}
        </Clickable>
      </View>
    };
  };

let make = (~children as renderContent, ~model, ~onClick, ~tree, ()) => {
  <nodeView renderContent model onClick node=tree />;
};
