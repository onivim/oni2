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
type activeIndentRange = {
  start: int,
  stop: int,
};

[@deriving show]
type model('node, 'leaf) = {
  rowHeight: int,
  activeIndentRange: option(activeIndentRange),
  treeAsList: Component_VimList.model(TreeList.t('node, 'leaf)),
};

let count = ({treeAsList, _}) => Component_VimList.count(treeAsList);

let create = (~rowHeight) => {
  activeIndentRange: None,
  rowHeight,
  treeAsList: Component_VimList.create(~rowHeight),
};

module Constants = {
  let arrowSize = 15.;
  let arrowSizeI = 15;
  let indentSize = 12;
  let scrollWheelMultiplier = 25;
  let scrollBarThickness = 6;
  //  let scrollTrackColor = Color.rgba(0.0, 0.0, 0.0, 0.4);
  //  let scrollThumbColor = Color.rgba(0.5, 0.5, 0.5, 0.4);
};

// UPDATE

type outmsg('node, 'leaf) =
  | Nothing
  | Expanded('node)
  | Collapsed('node)
  | Selected('leaf);

let calculateIndentGuides = model => {
  let focusedIndex = model.treeAsList |> Component_VimList.focusedIndex;

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
    |> Component_VimList.get(focusedIndex)
    |> OptionEx.flatMap(item => {
         switch (item) {
         | TreeList.ViewNode(_) => None
         | TreeList.ViewLeaf(_) =>
           let start = travel(~direction=-1, ~iteration=0, focusedIndex);
           let stop = travel(~direction=1, ~iteration=0, focusedIndex);
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

let update = (msg, model) => {
  switch (msg) {
  | List(listMsg) =>
    let (treeAsList, outmsg) =
      Component_VimList.update(listMsg, model.treeAsList);

    let model = {...model, treeAsList} |> calculateIndentGuides;

    switch (outmsg) {
    | Component_VimList.Nothing  => (model, Nothing);
    | Component_VimList.Selected({index}) =>
      switch (Component_VimList.get(index, treeAsList)) {
      
      | Some(ViewLeaf({data, _})) => (model, Selected(data))
      // TODO: Expand / collapse
      | Some(ViewNode(_)) 
      | None => (model, Nothing)
      }
    }

  };
};

let set = (trees: list(Tree.t('node, 'leaf)), model: model('node, 'leaf)) => {
  let treeAsList =
    trees |> List.map(TreeList.ofTree) |> List.flatten |> Array.of_list;

  {
    ...model,
    treeAsList: Component_VimList.set(treeAsList, model.treeAsList),
  };
};

module Contributions = {
  let commands =
    Component_VimList.Contributions.commands
    |> List.map(Oni_Core.Command.map(msg => List(msg)));
  let contextKeys = Component_VimList.Contributions.contextKeys;
};

module View = {
  open Revery.UI;
  module Codicon = Oni_Components.Codicon;
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
  let make = (~theme, ~model, ~dispatch, ~render, unit) => {
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
      theme
      model={model.treeAsList}
      dispatch={msg => dispatch(List(msg))}
      render={(~availableWidth, ~index, ~hovered, ~focused, item) => {
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
                ~focused,
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
                ~focused,
                Node({expanded, indentation: indentationLevel, data}),
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
