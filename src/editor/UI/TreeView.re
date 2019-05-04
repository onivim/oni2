open Oni_Model;
open Revery_UI;
open Revery.UI.Components;

module Core = Oni_Core;

type fsNode('a) = {
  displayName: string,
  path: string,
  isDirectory: bool,
  children: list('a),
  icon: option(IconTheme.IconDefinition.t),
  secondaryIcon: option(IconTheme.IconDefinition.t),
};

type treeItem =
  | FileSystemNode(fsNode(treeItem));

let component = React.component("TreeView");

let itemStyles = Style.[flexDirection(`Row), marginVertical(5)];

let toIcon = (~character, ~color) =>
  IconTheme.IconDefinition.{fontCharacter: character, fontColor: color};

let itemRenderer =
    (
      ~indent,
      ~onClick,
      font,
      itemFontSize,
      {data, status, id}: Tree.content(treeItem),
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

  let icon =
    switch (data) {
    | FileSystemNode({icon, secondaryIcon}) =>
      let makeIcon = toIcon(~color=Colors.white);
      switch (icon, secondaryIcon, isOpen) {
      | (Some(primary), Some(secondary), true) => secondary
      | (Some(primary), Some(secondary), false) => primary
      | (Some(primary), None, _) => primary
      | (None, Some(secondary), _) => secondary
      | (None, None, true) => makeIcon(~character=FontAwesome.caretDown)
      | (None, None, false) => makeIcon(~character=FontAwesome.caretRight)
      };
    };

  let fontFamily =
    switch (data) {
    | FileSystemNode({isDirectory: true}) => "FontAwesome5FreeSolid.otf"
    | FileSystemNode({isDirectory: false}) => "seti.ttf"
    };

  let label =
    switch (data) {
    | FileSystemNode({displayName}) => displayName
    };

  <Clickable onClick={() => onClick(id)}>
    <View style=itemStyles>
      <Text text=indentStr style=textStyles />
      <FontIcon
        fontFamily
        color={icon.fontColor}
        icon={icon.fontCharacter}
        backgroundColor=Colors.transparentWhite
      />
      <Text text=label style=Style.[marginLeft(10), ...textStyles] />
    </View>
  </Clickable>;
};

let containerStyles = (_theme: Core.Theme.t) =>
  Style.[padding(20), overflow(`Hidden), flexGrow(1)];

let titleStyles = (theme: Core.Theme.t, font) =>
  Style.[padding(5), fontSize(14), fontFamily(font)];

let headingStyles = (theme: Core.Theme.t) =>
  Style.[
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(theme.colors.editorBackground),
    paddingTop(5),
  ];

let toggleStatus = status =>
  Tree.(
    switch (status) {
    | Open => Closed
    | Closed => Open
    }
  );

let rec updateNode = (tree, nodeId, ~found) => {
  Tree.(
    switch (tree) {
    | Node({id, status, _} as data, children) when id == nodeId =>
      let node = Node({...data, status: toggleStatus(status)}, children);
      found := node;
      node;
    | Node(data, children) =>
      let newChildren =
        List.map(node => updateNode(node, nodeId, ~found), children);
      Node(data, newChildren);
    | Empty => Empty
    }
  );
};

let createElement =
    (
      ~children,
      ~title,
      ~tree: Tree.tree(treeItem),
      ~onNodeClick,
      ~state: State.t,
      (),
    ) =>
  component(hooks => {
    let itemFontSize = 12;
    let font = state.uiFont.fontFile;
    let {State.theme} = state;

    let found = ref(Tree.Empty);
    let onClick = id => updateNode(tree, id, ~found) |> onNodeClick(found^);

    (
      hooks,
      <View style=Style.[flexGrow(1)]>
        <View style={headingStyles(theme)}>
          <Text text=title style={titleStyles(theme, font)} />
        </View>
        <ScrollView style={containerStyles(theme)}>
          <Tree
            tree
            nodeRenderer={itemRenderer(~onClick, font, itemFontSize)}
          />
        </ScrollView>
      </View>,
    );
  });
