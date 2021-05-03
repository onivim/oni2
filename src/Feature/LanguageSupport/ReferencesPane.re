open EditorCoreTypes;
open Oni_Core;

type location = {
  path: string,
  ranges: list(CharacterRange.t),
};

type model = {
  locationNodes: list(Tree.t(location, Oni_Components.LocationListItem.t)),
  locationsView:
    Component_VimTree.model(location, Oni_Components.LocationListItem.t),
};

let initial = {
  locationNodes: [],
  locationsView: Component_VimTree.create(~rowHeight=20),
};

[@deriving show]
type msg =
  | LocationTree(Component_VimTree.msg)
  | KeyPress(string);

type outmsg =
  Component_VimTree.outmsg(location, Oni_Components.LocationListItem.t);

let locationsToReferences = (locations: list(Exthost.Location.t)) => {
  let map =
    List.fold_left(
      (acc, curr: Exthost.Location.t) => {
        let {range, uri}: Exthost.Location.t = curr;

        let range = range |> Exthost.OneBasedRange.toRange;

        let path = Oni_Core.Uri.toFileSystemPath(uri);

        acc
        |> StringMap.update(
             path,
             fun
             | None => Some([range])
             | Some(ranges) => Some([range, ...ranges]),
           );
      },
      StringMap.empty,
      locations,
    );

  map |> StringMap.bindings |> List.map(((path, ranges)) => {path, ranges});
};

let updateLocationTree = (nodes, model) => {
  let locationsView' =
    Component_VimTree.set(
      ~uniqueId=({path, _}) => path,
      ~searchText=
        Component_VimTree.(
          fun
          | Node({data, _}) => data.path
          | Leaf({data, _}) => Oni_Components.LocationListItem.(data.text)
        ),
      nodes,
      model.locationsView,
    );

  {locationNodes: nodes, locationsView: locationsView'};
};

let expandLocation = (~filePath, ~lines, model) => {
  let characterIndexToIndex = (idx: CharacterIndex.t) => {
    idx |> CharacterIndex.toInt |> Index.fromZeroBased;
  };

  let expandChildren = (location: location) => {
    let lineCount = Array.length(lines);
    location.ranges
    |> List.filter_map((range: CharacterRange.t) => {
         let line = range.start.line |> EditorCoreTypes.LineNumber.toZeroBased;
         if (line >= 0 && line < lineCount) {
           let highlight =
             if (range.start.line == range.stop.line) {
               Some((
                 range.start.character |> characterIndexToIndex,
                 range.stop.character |> characterIndexToIndex,
               ));
             } else {
               None;
             };
           Some(
             Oni_Components.LocationListItem.{
               file: filePath,
               location: range.start,
               text: lines[line],
               highlight,
             },
           );
         } else {
           None;
         };
       })
    |> List.sort(
         (
           a: Oni_Components.LocationListItem.t,
           b: Oni_Components.LocationListItem.t,
         ) => {
         (a.location.line |> EditorCoreTypes.LineNumber.toZeroBased)
         - (b.location.line |> EditorCoreTypes.LineNumber.toZeroBased)
       });
  };

  let locationNodes' =
    model.locationNodes
    |> List.map(
         fun
         | Tree.Leaf(_) as leaf => leaf
         | Tree.Node({data, _} as prev) =>
           if (data.path == filePath) {
             let children = data |> expandChildren |> List.map(Tree.leaf);

             Node({expanded: true, data, children});
           } else {
             Node(prev);
           },
       );

  model |> updateLocationTree(locationNodes');
};

let collapseLocations = model => {
  ...model,
  locationsView: Component_VimTree.collapse(model.locationsView),
};

let setLocations = (~maybeActiveBuffer, ~locations, model) => {
  let references = locationsToReferences(locations);

  let nodes =
    references
    |> List.map(reference =>
         Tree.node(~children=[], ~expanded=false, reference)
       );

  let model' =
    model
    // Un-expand all the nodes
    |> collapseLocations
    |> updateLocationTree(nodes);

  // Try to expand the current buffer, if it's available
  maybeActiveBuffer
  |> Utility.OptionEx.flatMap(buffer => {
       switch (Buffer.getFilePath(buffer)) {
       | None => None
       | Some(filePath) =>
         let lines = Buffer.getLines(buffer);
         Some(model' |> expandLocation(~filePath, ~lines));
       }
     })
  |> Option.value(~default=model');
};

let update = (msg, model) => {
  (model, Component_VimTree.Nothing);
};

module PaneView = {
  open Revery.UI;
  open Oni_Components;

  module Colors = Feature_Theme.Colors;

  module Styles = {
    open Style;

    let pane = [flexGrow(1), flexDirection(`Row)];

    let noResultsContainer = [
      flexGrow(1),
      alignItems(`Center),
      justifyContent(`Center),
    ];

    let title = (~theme) => [
      color(Colors.PanelTitle.activeForeground.from(theme)),
      margin(8),
    ];
  };

  let make =
      (
        ~config,
        ~isFocused: bool,
        ~locationsList:
           Component_VimTree.model(
             location,
             Oni_Components.LocationListItem.t,
           ),
        ~theme,
        ~iconTheme,
        ~languageInfo,
        ~uiFont: UiFont.t,
        ~workingDirectory,
        ~dispatch: Component_VimTree.msg => unit,
        (),
      ) => {
    let innerElement =
      if (Component_VimTree.count(locationsList) == 0) {
        <View style=Styles.noResultsContainer>
          <Text
            style={Styles.title(~theme)}
            fontFamily={uiFont.family}
            fontSize={uiFont.size}
            text="No locations set."
          />
        </View>;
      } else {
        <Component_VimTree.View
          config
          font=uiFont
          isActive=isFocused
          focusedIndex=None
          theme
          model=locationsList
          dispatch
          render={(
            ~availableWidth,
            ~index as _,
            ~hovered as _,
            ~selected as _,
            item,
          ) =>
            switch (item) {
            | Component_VimTree.Node({data, _}) =>
              <FileItemView.View
                theme
                uiFont
                iconTheme
                languageInfo
                item={data.path}
                workingDirectory
              />
            | Component_VimTree.Leaf({data, _}) =>
              <LocationListItem.View
                showPosition=true
                width=availableWidth
                theme
                uiFont
                item=data
              />
            }
          }
        />;
      };

    <View style=Styles.pane> innerElement </View>;
  };
};

let contextKeys = (~isFocused, model) =>
  if (isFocused) {
    Component_VimTree.Contributions.contextKeys(model.locationsView);
  } else {
    WhenExpr.ContextKeys.empty;
  };

open Feature_Pane.Schema;
let pane: Feature_Pane.Schema.t(model, msg) =
  panel(
    ~title="Locations",
    ~id=Some("workbench.panel.locations"),
    ~commands=
      pane => {
        Component_VimTree.Contributions.commands
        |> List.map(Oni_Core.Command.map(msg => LocationTree(msg)))
      },
    ~contextKeys,
    ~view=
      (
        ~config,
        ~font,
        ~isFocused,
        ~iconTheme,
        ~languageInfo,
        ~workingDirectory,
        ~theme,
        ~dispatch,
        ~model,
      ) => {
        <PaneView
          uiFont=font
          config
          isFocused
          locationsList={model.locationsView}
          theme
          iconTheme
          languageInfo
          workingDirectory
          dispatch={msg => dispatch(LocationTree(msg))}
        />
      },
    ~keyPressed=key => KeyPress(key),
  );
