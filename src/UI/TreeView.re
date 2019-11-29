open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Model.UiTree;

module Styles = {
  open Style;

  let clickable = [
    cursor(Revery.MouseCursors.pointer),
    flexDirection(`Row),
    marginVertical(3),
  ];
  let children = [transform(Transform.[TranslateX(10.)])];
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
        fontFamily="FontAwesome5FreeSolid.otf"
        fontSize=15
        color=Colors.white
        icon={node.isOpen ? FontAwesome.caretDown : FontAwesome.caretRight}
        backgroundColor=Colors.transparentWhite
      />;
    } else {
      React.empty;
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
