open Oni_Model;
open Revery_UI;
open Revery.UI.Components;

module Core = Oni_Core;

let component = React.component("FileExplorer");

let itemStyles = Style.[flexDirection(`Row), marginVertical(5)];

let itemRenderer =
    (
      ~indent,
      ~onClick,
      font,
      itemFontSize,
      {data, status, id}: Tree.content('a),
    ) => {
  open Tree;
  open Revery;

  let isOpen =
    switch (status) {
    | Open => true
    | Closed => false
    };

  let textStyles =
    Style.[fontSize(itemFontSize), fontFamily(font), color(Colors.white)];

  let indentStr = String.make(indent * 2, ' ');
  let arrow = isOpen ? FontAwesome.sortDown : FontAwesome.sortUp;

  <Clickable onClick={() => onClick(id)}>
    <View style=itemStyles>
      <Text text=indentStr style=textStyles />
      <FontIcon
        icon=arrow
        backgroundColor=Colors.transparentWhite
        color=Colors.white
      />
      <Text text=data style=Style.[marginLeft(10), ...textStyles] />
    </View>
  </Clickable>;
};

let containerStyles = (_theme: Core.Theme.t) =>
  Style.[padding(20), overflow(`Hidden), flexGrow(1)];

let titleStyles = (theme: Core.Theme.t, font) =>
  Style.[padding(5), fontSize(14), fontFamily(font)];

let toggleStatus = status =>
  Tree.(
    switch (status) {
    | Open => Closed
    | Closed => Open
    }
  );

let rec updateNode = (tree, nodeId) => {
  Tree.(
    switch (tree) {
    | Node({id, status, _} as data, children) when id == nodeId =>
      Node({...data, status: toggleStatus(status)}, children)
    | Node(data, children) =>
      let newChildren = List.map(node => updateNode(node, nodeId), children);
      Node(data, newChildren);
    | Empty => Empty
    }
  );
};

let createElement = (~children, ~title, ~tree, ~state: State.t, ()) =>
  component(hooks => {
    let itemFontSize = 12;
    let font = state.uiFont.fontFile;
    let {State.theme} = state;
    let (stateTree, setTree, hooks) = React.Hooks.state(tree, hooks);

    let onClick = id => updateNode(stateTree, id) |> setTree;

    (
      hooks,
      <View style=Style.[flexGrow(1)]>
        <View
          style=Style.[
            flexDirection(`Row),
            justifyContent(`Center),
            alignItems(`Center),
            backgroundColor(theme.colors.editorBackground),
            paddingTop(5),
          ]>
          <Text text=title style={titleStyles(theme, font)} />
        </View>
        <ScrollView style={containerStyles(theme)}>
          <Tree
            tree=stateTree
            nodeRenderer={itemRenderer(~onClick, font, itemFontSize)}
          />
        </ScrollView>
      </View>,
    );
  });
