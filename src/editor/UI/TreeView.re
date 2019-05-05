open Oni_Model;
open UiTree;
open Revery_UI;
open Revery.UI.Components;

module Core = Oni_Core;

let component = React.component("TreeView");

let itemStyles = Style.[flexDirection(`Row), marginVertical(5)];

let containerStyles = Style.[padding(20), overflow(`Hidden), flexGrow(1)];

let titleStyles = font =>
  Style.[padding(5), fontSize(14), fontFamily(font)];

let headingStyles = (theme: Core.Theme.t) =>
  Style.[
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(theme.colors.editorBackground),
    paddingTop(5),
  ];

let toIcon = (~character, ~color) =>
  IconTheme.IconDefinition.{fontCharacter: character, fontColor: color};

let itemRenderer =
    (
      ~font,
      ~indent,
      ~onClick,
      ~primaryRootIcon,
      ~secondaryRootIcon,
      ~itemSize,
      {data, status, id}: itemContent,
    ) => {
  open Revery;

  let isOpen =
    switch (status) {
    | Open => true
    | Closed => false
    };

  let textStyles =
    Style.[fontSize(itemSize), fontFamily(font), color(Colors.white)];

  let indentStr = String.make(indent * 2, ' ');

  let icon =
    switch (data) {
    | FileSystemNode({icon, secondaryIcon, _}) =>
      let makeIcon = toIcon(~color=Colors.white);
      switch (icon, secondaryIcon, isOpen) {
      | (Some(_), Some(secondary), true) => secondary
      | (Some(primary), Some(_), false) => primary
      | (Some(primary), None, _) => primary
      | (None, Some(secondary), _) => secondary
      | (None, None, false) => makeIcon(~character=primaryRootIcon)
      | (None, None, true) => makeIcon(~character=secondaryRootIcon)
      };
    };

  let fontFamily =
    switch (data) {
    | FileSystemNode({isDirectory: true, _}) => "FontAwesome5FreeSolid.otf"
    | FileSystemNode({isDirectory: false, _}) => "seti.ttf"
    };

  let label =
    switch (data) {
    | FileSystemNode({displayName, _}) => displayName
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

let toggleStatus = status =>
  switch (status) {
  | Open => Closed
  | Closed => Open
  };

let updateNode = (nodeId, tree) => {
  let updatedNode = ref(Empty);

  let rec update = (nodeId, tree) => {
    switch (tree) {
    | Node({id, status, _} as data, children) when id == nodeId =>
      let node = Node({...data, status: toggleStatus(status)}, children);
      /*
       Store a reference to the located/updated node
       TODO: find a solution that doesn't require a ref
       */
      updatedNode := node;
      node;
    | Node(data, children) =>
      let newChildren = List.map(update(nodeId), children);
      Node(data, newChildren);
    | Empty => Empty
    };
  };

  {updated: updatedNode^, tree: update(nodeId, tree)};
};

let createElement =
    (
      ~title,
      ~children as _,
      ~tree: UiTree.t,
      ~onNodeClick,
      ~state: State.t,
      ~primaryRootIcon=FontAwesome.caretRight,
      ~secondaryRootIcon=FontAwesome.caretDown,
      (),
    ) =>
  component(hooks => {
    let itemSize = 12;
    let font = state.uiFont.fontFile;
    let {State.theme, _} = state;

    let onClick = id =>
      updateNode(id, tree)
      |> (({updated, tree, _}) => onNodeClick(updated, tree));

    (
      hooks,
      <View style=Style.[flexGrow(1)]>
        <View style={headingStyles(theme)}>
          <Text text=title style={titleStyles(font)} />
        </View>
        <ScrollView style=containerStyles>
          <Tree
            tree
            nodeRenderer={itemRenderer(
              ~onClick,
              ~primaryRootIcon,
              ~secondaryRootIcon,
              ~font,
              ~itemSize,
            )}
          />
        </ScrollView>
      </View>,
    );
  });
