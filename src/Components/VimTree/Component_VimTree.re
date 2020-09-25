open Oni_Core;
open Utility;

[@deriving show]
type msg =
  | List(Component_VimList.msg);

type nodeOrLeaf('node, 'leaf) =
  | Node({
      expanded: bool,
      indentation: int,
      data: 'node,
    })
  | Leaf({
      indentation: int,
      data: 'leaf,
    });

[@deriving show]
type withUniqueId('a) = {
  uniqueId: string,
  inner: 'a,
};

module ExpansionContext = {
  type expansion = {
    originalExpanded: bool,
    expanded: bool,
  };

  type t = StringMap.t(expansion);

  let initial = StringMap.empty;

  let overrideExpansions = (expansionContext: t, tree) => {
    tree
    |> Tree.setExpanded((~current, node) => {
         StringMap.find_opt(node.uniqueId, expansionContext)
         |> Option.map(({expanded, _}) => expanded)
         |> Option.value(~default=current)
       });
  };

  let expand = (uniqueId, expansionContext) => {
    StringMap.update(
      uniqueId,
      Option.map(expansion => {...expansion, expanded: true}),
      expansionContext,
    );
  };

  let collapse = (uniqueId, expansionContext) => {
    StringMap.update(
      uniqueId,
      Option.map(expansion => {...expansion, expanded: false}),
      expansionContext,
    );
  };

  // Update our expansion context - if any expansions have changed,
  // remove them.
  let update = (expansionContext: t, tree) => {
    tree
    |> Tree.fold(
         (acc, curr) => {
           switch (curr) {
           | Tree.Leaf(_) => acc
           | Tree.Node(node) =>
             StringMap.update(
               node.data.uniqueId,
               fun
               | None =>
                 Some({
                   originalExpanded: node.expanded,
                   expanded: node.expanded,
                 })
               | Some(info) when info.originalExpanded == node.expanded =>
                 Some(info)
               | Some(_) =>
                 Some({
                   originalExpanded: node.expanded,
                   expanded: node.expanded,
                 }),
               acc,
             )
           }
         },
         expansionContext,
       );
  };
};

[@deriving show]
type activeIndentRange = {
  start: int,
  stop: int,
};

[@deriving show]
type model('node, 'leaf) = {
  expansionContext: [@opaque] ExpansionContext.t,
  [@deriving show]
  rowHeight: int,
  activeIndentRange: option(activeIndentRange),
  trees: list(Tree.t(withUniqueId('node), 'leaf)),
  treeAsList:
    Component_VimList.model(TreeList.t(withUniqueId('node), 'leaf)),
};

let count = ({treeAsList, _}) => Component_VimList.count(treeAsList);

let findIndex = (f, {treeAsList, _}) => {
  let pred =
    fun
    | TreeList.ViewNode({expanded, indentationLevel, data}) =>
      f(Node({expanded, indentation: indentationLevel, data: data.inner}))
    | TreeList.ViewLeaf({indentationLevel, data}) =>
      f(Leaf({indentation: indentationLevel, data}));

  Component_VimList.findIndex(pred, treeAsList);
};

let scrollTo = (~index, ~alignment, {treeAsList, _} as model) => {
  {
    ...model,
    treeAsList: Component_VimList.scrollTo(~index, ~alignment, treeAsList),
  };
};

let create = (~rowHeight) => {
  expansionContext: ExpansionContext.initial,
  activeIndentRange: None,
  rowHeight,
  trees: [],
  treeAsList: Component_VimList.create(~rowHeight),
};

module Constants = {
  let arrowSize = 15.;
  let indentSize = 12;
};

// UPDATE

type outmsg('node, 'leaf) =
  | Nothing
  | Expanded('node)
  | Collapsed('node)
  | Selected('leaf);

let calculateIndentGuides = model => {
  let selectedIndex = model.treeAsList |> Component_VimList.selectedIndex;

  let count = model.treeAsList |> Component_VimList.count;
  let rec travel = (~direction, ~iteration, idx) =>
    if (idx + direction <= 0 || idx + direction >= count || iteration > 250) {
      idx;
    } else {
      switch (Component_VimList.get(idx + direction, model.treeAsList)) {
      | Some(ViewLeaf(_)) =>
        travel(~direction, ~iteration=iteration + 1, idx + direction)
      | Some(ViewNode(_)) => idx
      | None => idx
      };
    };

  let activeIndentRange =
    model.treeAsList
    |> Component_VimList.get(selectedIndex)
    |> OptionEx.flatMap(item => {
         switch (item) {
         | TreeList.ViewNode(_) => None
         | TreeList.ViewLeaf(_) =>
           let start = travel(~direction=-1, ~iteration=0, selectedIndex);
           let stop = travel(~direction=1, ~iteration=0, selectedIndex);
           Some({start, stop});
         }
       });
  {...model, activeIndentRange};
};

let isActiveIndent = (index, model) => {
  model.activeIndentRange
  |> Option.map(range => {index >= range.start && index <= range.stop})
  |> Option.value(~default=false);
};

let updateTreeList = (treesWithId, expansionContext, model) => {
  let trees =
    treesWithId
    |> List.map(ExpansionContext.overrideExpansions(expansionContext))
    |> List.map(TreeList.ofTree)
    |> List.flatten
    |> Array.of_list;

  {
    ...model,
    expansionContext,
    treeAsList: Component_VimList.set(trees, model.treeAsList),
  };
};

let update = (msg, model) => {
  switch (msg) {
  | List(listMsg) =>
    let (treeAsList, outmsg) =
      Component_VimList.update(listMsg, model.treeAsList);

    let model = {...model, treeAsList} |> calculateIndentGuides;

    switch (outmsg) {
    | Component_VimList.Nothing => (model, Nothing)
    | Component_VimList.Selected({index}) =>
      switch (Component_VimList.get(index, treeAsList)) {
      | Some(ViewLeaf({data, _})) => (model, Selected(data))
      // TODO: Expand / collapse
      | Some(ViewNode({data, expanded, _})) =>
        let expansionContext =
          expanded
            ? model.expansionContext
              |> ExpansionContext.collapse(data.uniqueId)
            : model.expansionContext |> ExpansionContext.expand(data.uniqueId);

        (
          updateTreeList(model.trees, expansionContext, model),
          expanded ? Collapsed(data.inner) : Expanded(data.inner),
        );

      | None => (model, Nothing)
      }
    };
  };
};

let set =
    (
      ~uniqueId,
      trees: list(Tree.t('node, 'leaf)),
      model: model('node, 'leaf),
    ) => {
  // Tag the trees with an ID
  let treesWithId =
    trees
    |> List.map(
         Tree.map(
           ~leaf=v => v,
           ~node=data => {uniqueId: uniqueId(data), inner: data},
         ),
       );

  // Clear out any expansions that have changed
  let expansionContext =
    treesWithId
    |> List.fold_left(
         (acc: ExpansionContext.t, curr) => {
           ExpansionContext.update(acc, curr)
         },
         model.expansionContext,
       );

  {
    ...updateTreeList(treesWithId, expansionContext, model),
    trees: treesWithId,
  };
};

module Contributions = {
  let commands =
    Component_VimList.Contributions.commands
    |> List.map(Oni_Core.Command.map(msg => List(msg)));
  let contextKeys = model =>
    Component_VimList.Contributions.contextKeys(model.treeAsList);
};

module View = {
  open Revery.UI;
  module Colors = Feature_Theme.Colors;

  module Styles = {
    open Style;
    // Margin applied to center vertically
    let arrow = size => [
      width(size),
      height(size),
      marginTop(4),
      marginRight(4),
    ];
  };

  let indentGuide = (~horizontalSize, ~verticalSize, ~strokeColor) => {
    <View
      style=Style.[
        marginLeft(horizontalSize / 2 + 1),
        width(horizontalSize / 2 - 1),
        height(verticalSize),
        borderLeft(~color=strokeColor, ~width=1),
      ]
    />;
  };

  let indent = (~level, ~width, ~height, ~color) => {
    List.init(level, _ =>
      indentGuide(
        ~horizontalSize=width,
        ~verticalSize=height,
        ~strokeColor=color,
      )
    );
  };

  let arrow = (~isOpen, ~color, ()) =>
    <View style={Styles.arrow(int_of_float(Constants.arrowSize))}>
      <Codicon
        icon={isOpen ? Codicon.chevronDown : Codicon.chevronRight}
        color
        fontSize=Constants.arrowSize
      />
    </View>;
  let make =
      (~isActive, ~focusedIndex, ~theme, ~model, ~dispatch, ~render, ()) => {
    let indentHeight = model.rowHeight;
    let indentWidth = Constants.indentSize;
    let activeIndentColor = Colors.List.activeIndentGuide.from(theme);
    let inactiveIndentColor = Colors.List.inactiveIndentGuide.from(theme);

    let makeIndent = (~isActive, level) => {
      indent(
        ~level,
        ~width=indentWidth,
        ~height=indentHeight,
        ~color=isActive ? activeIndentColor : inactiveIndentColor,
      )
      |> React.listToElement;
    };

    <Component_VimList.View
      isActive
      focusedIndex
      theme
      model={model.treeAsList}
      dispatch={msg => dispatch(List(msg))}
      render={(~availableWidth, ~index, ~hovered, ~selected, item) => {
        // Render actual item
        let innerView =
          switch (item) {
          | TreeList.ViewLeaf({indentationLevel, data}) => [
              makeIndent(
                ~isActive=isActiveIndent(index, model),
                indentationLevel,
              ),
              render(
                ~availableWidth,
                ~index,
                ~hovered,
                ~selected,
                Leaf({indentation: indentationLevel, data}),
              ),
            ]

          | TreeList.ViewNode({expanded, indentationLevel, data}) =>
            let icon =
              arrow(
                ~isOpen=expanded,
                ~color=Colors.SideBar.foreground.from(theme),
                (),
              );

            [
              makeIndent(~isActive=false, indentationLevel),
              icon,
              render(
                ~availableWidth,
                ~index,
                ~hovered,
                ~selected,
                Node({
                  expanded,
                  indentation: indentationLevel,
                  data: data.inner,
                }),
              ),
            ];
          };
        <View style=Style.[flexDirection(`Row)]>
          {innerView |> React.listToElement}
        </View>;
      }}
    />;
  };
};
