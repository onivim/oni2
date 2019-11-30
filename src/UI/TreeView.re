open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Model.UiTree;

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
};

let rec nodeView = (~renderContent, ~onClick, ~node, ()) => {
  let children =
    if (node.isOpen) {
      List.map(
        child => <nodeView renderContent onClick node=child />,
        node.children,
      );
    } else {
      [];
    };

  let arrow = () =>
    if (node.data.isDirectory) {
      <FontIcon
        fontSize=Constants.arrowSize
        color=Colors.white
        icon={node.isOpen ? FontAwesome.caretDown : FontAwesome.caretRight}
        backgroundColor=Colors.transparentWhite
      />;
    } else {
      <View
        style=Style.[
          width(Constants.arrowSize),
          height(Constants.arrowSize),
        ]
      />;
    };

  <View>
    <Clickable onClick={() => onClick(node)} style=Styles.clickable>
      <arrow />
      {renderContent(node)}
    </Clickable>
    <View style=Styles.children> {children |> React.listToElement} </View>
  </View>;
};

let make = (~children as renderContent, ~onClick, ~tree, ()) => {
  <nodeView renderContent onClick node=tree />;
};
