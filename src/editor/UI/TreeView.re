open Oni_Model;
open Revery_UI;
open Revery.UI.Components;

module Core = Oni_Core;

type fsNode('a) = {
  displayName: string,
  fullPath: string,
  isDirectory: bool,
  children: list('a),
  icon: option(IconTheme.IconDefinition.t),
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
    | FileSystemNode({icon}) =>
      let makeIcon = toIcon(~color=Colors.white);
      switch (icon) {
      | Some(i) => i
      | None =>
        isOpen
          ? makeIcon(~character=FontAwesome.sortDown)
          : makeIcon(~character=FontAwesome.sortUp)
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

let rec logTree = t => {
  Tree.(
    switch (t) {
    | Node({data, _}, children) =>
      switch (data) {
      | FileSystemNode({displayName, _}) =>
        print_endline("Name: " ++ displayName)
      };
      print_endline(
        "No. of Siblings: " ++ (List.length(children) |> string_of_int),
      );
      List.iter(logTree, children);
    | Empty => print_endline("Nothing!!!!")
    }
  );
};

let createElement =
    (~children, ~title, ~tree: Tree.tree(treeItem), ~state: State.t, ()) =>
  component(hooks => {
    let itemFontSize = 12;
    let font = state.uiFont.fontFile;
    let {State.theme} = state;

    let (stateTree, setTree, hooks) = React.Hooks.state(tree, hooks);
    logTree(stateTree);

    let onClick = id => updateNode(stateTree, id) |> setTree;

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
