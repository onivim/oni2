open Oni_Core;
open Utility;

[@deriving show]
type command =
  | Select
  | ToggleExpanded;

[@deriving show]
type msg =
  | Command(command)
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
  level: int,
  start: int,
  stop: int,
};

[@deriving show]
type model('node, 'leaf) = {
  expansionContext: [@opaque] ExpansionContext.t,
  [@deriving show]
  rowHeight: int,
  activeIndentRange: option(activeIndentRange),
  maybeSearchFunction:
    [@opaque] option(TreeList.t(withUniqueId('node), 'leaf) => string),
  trees: list(Tree.t(withUniqueId('node), 'leaf)),
  treeAsList:
    Component_VimList.model(TreeList.t(withUniqueId('node), 'leaf)),
};

let count = ({treeAsList, _}) => Component_VimList.count(treeAsList);

let treeListItemToNodeOrLeaf =
  fun
  | TreeList.ViewNode({expanded, indentationLevel, data}) =>
    Node({expanded, indentation: indentationLevel, data: data.inner})
  | TreeList.ViewLeaf({indentationLevel, data}) =>
    Leaf({indentation: indentationLevel, data});

let findIndex = (f, {treeAsList, _}) => {
  let pred = item => {
    f(treeListItemToNodeOrLeaf(item));
  };

  Component_VimList.findIndex(pred, treeAsList);
};

let setSelected = (~selected, {treeAsList, _} as model) => {
  let treeAsList' = treeAsList |> Component_VimList.setSelected(~selected);
  {...model, treeAsList: treeAsList'};
};

let getNextNodeFromPosition = (~direction=1, ~position, {treeAsList, _}) => {
  let count = Component_VimList.count(treeAsList);

  let rec loop = (~start, ~offset) => {
    let idx = IntEx.modulo(start + offset, count);
    if (idx < 0 || idx >= count) {
      None;
    } else {
      switch (Component_VimList.get(idx, treeAsList)) {
      | None => loop(~start, ~offset=offset + direction)
      | Some(item) when !TreeList.isLeaf(item) =>
        loop(~start, ~offset=offset + direction)
      | Some(_item) => Some(idx)
      };
    };
  };

  loop(~start=position + direction, ~offset=0);
};

let selectNextNode = ({treeAsList, _} as model) => {
  let selectedIdx = Component_VimList.selectedIndex(treeAsList);
  let maybeNextSelected =
    getNextNodeFromPosition(~direction=1, ~position=selectedIdx, model);

  switch (maybeNextSelected) {
  | None => model
  | Some(newIndex) => setSelected(~selected=newIndex, model)
  };
};

let selectPreviousNode = ({treeAsList, _} as model) => {
  let selectedIdx = Component_VimList.selectedIndex(treeAsList);
  let maybePreviousSelected =
    getNextNodeFromPosition(~direction=-1, ~position=selectedIdx, model);

  switch (maybePreviousSelected) {
  | None => model
  | Some(newIndex) => setSelected(~selected=newIndex, model)
  };
};

let selected = ({treeAsList, _}) => {
  Component_VimList.selected(treeAsList)
  |> Option.map(treeListItemToNodeOrLeaf);
};

let keyPress = (key, {treeAsList, _} as model) => {
  ...model,
  treeAsList: Component_VimList.keyPress(key, treeAsList),
};

let scrollTo = (~index, ~alignment, {treeAsList, _} as model) => {
  {
    ...model,
    treeAsList: Component_VimList.scrollTo(~index, ~alignment, treeAsList),
  };
};

let collapse = model => {
  ...model,
  expansionContext: ExpansionContext.initial,
};

let create = (~rowHeight) => {
  expansionContext: ExpansionContext.initial,
  activeIndentRange: None,
  rowHeight,
  trees: [],
  treeAsList: Component_VimList.create(~rowHeight),
  maybeSearchFunction: None,
};

module Constants = {
  let arrowSize = 15.;
  let indentCaretSize = 7;
};

// UPDATE

type outmsg('node, 'leaf) =
  | Nothing
  | Expanded('node)
  | Collapsed('node)
  | Touched('leaf)
  | Selected('leaf)
  | SelectedNode('node);

let calculateIndentGuides = model => {
  let selectedIndex = model.treeAsList |> Component_VimList.selectedIndex;

  let count = model.treeAsList |> Component_VimList.count;
  let rec travel = (~indentationLevel, ~direction, ~iteration, idx) =>
    if (idx + direction <= 0 || idx + direction >= count || iteration > 250) {
      idx;
    } else {
      switch (Component_VimList.get(idx + direction, model.treeAsList)) {
      | Some(ViewLeaf(_)) =>
        travel(
          ~indentationLevel,
          ~direction,
          ~iteration=iteration + 1,
          idx + direction,
        )
      | Some(ViewNode({indentationLevel: indent, _}))
          when indent < indentationLevel => idx
      | Some(ViewNode(_)) =>
        travel(
          ~indentationLevel,
          ~direction,
          ~iteration=iteration + 1,
          idx + direction,
        )
      | None => idx
      };
    };

  let activeIndentRange =
    model.treeAsList
    |> Component_VimList.get(selectedIndex)
    |> OptionEx.flatMap(item => {
         switch (item) {
         | TreeList.ViewNode({indentationLevel, _})
         | TreeList.ViewLeaf({indentationLevel, _}) =>
           let start =
             travel(
               ~indentationLevel,
               ~direction=-1,
               ~iteration=0,
               selectedIndex,
             );
           let stop =
             travel(
               ~indentationLevel,
               ~direction=1,
               ~iteration=0,
               selectedIndex,
             );
           Some({level: indentationLevel, start, stop});
         }
       });
  {...model, activeIndentRange};
};

let activeLevel = (index, model) => {
  model.activeIndentRange
  |> OptionEx.flatMap(range =>
       if (index >= range.start && index <= range.stop) {
         Some(range.level);
       } else {
         None;
       }
     );
};

let updateTreeList = (~searchText=?, treesWithId, expansionContext, model) => {
  let trees =
    treesWithId
    |> List.map(ExpansionContext.overrideExpansions(expansionContext))
    |> List.map(TreeList.ofTree)
    |> List.flatten
    |> Array.of_list;

  {
    ...model,
    expansionContext,
    treeAsList: Component_VimList.set(~searchText?, trees, model.treeAsList),
  };
};

let toggleExpanded = (~expanded, ~data: withUniqueId('a), model) => {
  let expansionContext =
    expanded
      ? model.expansionContext |> ExpansionContext.collapse(data.uniqueId)
      : model.expansionContext |> ExpansionContext.expand(data.uniqueId);

  (
    updateTreeList(model.trees, expansionContext, model),
    expanded ? Collapsed(data.inner) : Expanded(data.inner),
  );
};

let update:
  (msg, model('node, 'leaf)) => (model('node, 'leaf), outmsg('node, 'leaf)) =
  (msg, model) => {
    switch (msg) {
    | List(listMsg) =>
      let (treeAsList, outmsg) =
        Component_VimList.update(listMsg, model.treeAsList);

      let model = {...model, treeAsList} |> calculateIndentGuides;

      switch (outmsg) {
      | Component_VimList.Nothing => (model, Nothing)
      | Component_VimList.Touched({index}) =>
        switch (Component_VimList.get(index, treeAsList)) {
        | Some(ViewLeaf({data, _})) => (model, Touched(data))
        // TODO: Expand / collapse
        | Some(ViewNode({data, expanded, _})) =>
          toggleExpanded(~expanded, ~data, model)
        | None => (model, Nothing)
        }
      | Component_VimList.Selected({index}) =>
        switch (Component_VimList.get(index, treeAsList)) {
        | Some(ViewLeaf({data, _})) => (model, Selected(data))
        // TODO: Expand / collapse
        | Some(ViewNode({data, expanded, _})) =>
          toggleExpanded(~expanded, ~data, model)

        | None => (model, Nothing)
        }
      };

    | Command(Select) =>
      let selectedIndex = Component_VimList.selectedIndex(model.treeAsList);
      switch (Component_VimList.get(selectedIndex, model.treeAsList)) {
      | Some(ViewLeaf({data, _})) => (model, Selected(data))
      | Some(ViewNode({data, _})) => (model, SelectedNode(data.inner))
      | None => (model, Nothing)
      };

    | Command(ToggleExpanded) =>
      let selectedIndex = Component_VimList.selectedIndex(model.treeAsList);
      switch (Component_VimList.get(selectedIndex, model.treeAsList)) {
      | None => (model, Nothing)
      | Some(ViewLeaf(_)) => (model, Nothing)
      | Some(ViewNode({data, expanded, _})) =>
        toggleExpanded(~expanded, ~data, model)
      };
    };
  };

let set =
    (
      ~searchText=?,
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

  let maybeSearchFunction =
    searchText
    |> Option.map(search => {
         let f = (nodeOrLeaf: TreeList.t(withUniqueId('node), 'leaf)) =>
           switch (nodeOrLeaf) {
           | ViewNode({expanded, indentationLevel, data}) =>
             search(
               Node({
                 expanded,
                 indentation: indentationLevel,
                 data: data.inner,
               }),
             )
           | ViewLeaf({indentationLevel, data}) =>
             search(Leaf({indentation: indentationLevel, data}))
           };
         f;
       });

  {
    ...
      updateTreeList(
        ~searchText=?maybeSearchFunction,
        treesWithId,
        expansionContext,
        model,
      ),
    maybeSearchFunction,
    trees: treesWithId,
  };
};

module Commands = {
  open Feature_Commands.Schema;

  let toggleExpanded =
    define("vim.tree.toggleExpanded", Command(ToggleExpanded));

  let select = define("vim.tree.select", Command(Select));
};

module Keybindings = {
  let commandCondition =
    "!textInputFocus && vimListNavigation" |> WhenExpr.parse;

  let keybindings =
    Feature_Input.Schema.[
      bind(
        ~key="h",
        ~command=Commands.toggleExpanded.id,
        ~condition=commandCondition,
      ),
      bind(
        ~key="l",
        ~command=Commands.toggleExpanded.id,
        ~condition=commandCondition,
      ),
      bind(
        ~key="<space>",
        ~command=Commands.select.id,
        ~condition=commandCondition,
      ),
    ];
};

module Contributions = {
  let keybindings = Keybindings.keybindings;

  let commands =
    (
      Component_VimList.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => List(msg)))
    )
    @ Commands.[toggleExpanded, select];

  let contextKeys = model => {
    open WhenExpr.ContextKeys;

    let vimListKeys =
      Component_VimList.Contributions.contextKeys(model.treeAsList);
    let vimTreeKeys =
      [
        Schema.bool("vimTreeNavigation", ({treeAsList, _}) => {
          !(treeAsList |> Component_VimList.isSearchOpen)
        }),
      ]
      |> Schema.fromList
      |> fromSchema(model);

    [vimListKeys, vimTreeKeys] |> unionMany;
  };
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
    let offset = max(0, horizontalSize);
    <View
      style=Style.[
        marginLeft(Constants.indentCaretSize),
        width(offset),
        height(verticalSize),
        borderLeft(~color=strokeColor, ~width=1),
      ]
    />;
  };

  let indent =
      (~activeLevel, ~level, ~width, ~height, ~inactiveColor, ~activeColor) => {
    List.init(level, i =>
      indentGuide(
        ~horizontalSize=width,
        ~verticalSize=height,
        ~strokeColor=Some(i + 1) == activeLevel ? activeColor : inactiveColor,
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
      (
        ~config,
        ~isActive,
        ~font,
        ~focusedIndex,
        ~theme,
        ~model,
        ~dispatch,
        ~render,
        (),
      ) => {
    let indentHeight = model.rowHeight;
    let indentWidth =
      Feature_Configuration.GlobalConfiguration.Workbench.treeIndent.get(
        config,
      );
    let showIndentGuides =
      Feature_Configuration.GlobalConfiguration.Workbench.treeRenderIndentGuides.
        get(
        config,
      );
    let activeIndentColor =
      showIndentGuides
        ? Colors.List.activeIndentGuide.from(theme)
        : Revery.Colors.transparentBlack;
    let inactiveIndentColor =
      showIndentGuides
        ? Colors.List.inactiveIndentGuide.from(theme)
        : Revery.Colors.transparentBlack;

    let makeIndent = (~activeLevel, level) => {
      indent(
        ~activeLevel,
        ~level,
        ~width=indentWidth,
        ~height=indentHeight,
        ~inactiveColor=inactiveIndentColor,
        ~activeColor=activeIndentColor,
      )
      |> React.listToElement;
    };

    <Component_VimList.View
      isActive
      font
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
                ~activeLevel=activeLevel(index, model),
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
              makeIndent(
                ~activeLevel=activeLevel(index, model),
                indentationLevel,
              ),
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
